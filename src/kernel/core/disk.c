#include "disk.h"
#include "util/portio.h"
#include "util/logging.h"
#include "memory.h"
// This file contains an implementation for interfacing with ATA hard drives directly, through port IO.
// Note: this method is inherently slow and there exist other (much faster) solutions, namely DMA.

// Another, very crucial TODO: This approach DOES NOT consider multitasking whatsoever.
// Currently it's just a state machine where we only expect one read to be performed at
// a time, and we just block until any ongoing reads or writes are completed.
// 
// Really, we should be using some sort of queue, and performing other tasks while we wait
// instead of simply blocking.

uint16_t identify_primary[256] = {0};
uint16_t identify_secondary[256] = {0};

uint8_t* target_buffer = 0x0000; // The buffer we're writing disk data to (when reading from disk) or reading from (when writing to disk)
uint8_t sector_count = 0; // Number of sectors we are currently reading/writing.
uint8_t current_sector = 0; // Current sector we are reading/writing.

bool has_primary = FALSE;
bool has_secondary = FALSE;

disk_state state = IDLE;

bool disk_detect_floating() {
    uint8_t in = inb(ATA_PRIMARY_STATUS_CMD); // Read status byte
    return in == 0xFF;
}

void disk_init() {
    register_interrupt(ATA_PRIMARY_IRQ, (uint32_t)disk_primary_irq);
    register_interrupt(ATA_SECONDARY_IRQ, (uint32_t)disk_secondary_irq);

    if (disk_detect_floating()) {
        log_error("disk_init: err: no drives connected.");
        return;
    }

    disk_identify(0);
    disk_identify(1);

    log_disk_info();
}

void log_disk_info() {
    if (has_primary) {
        log_info("Primary disk:");
        bool supports_lba48 = (bool)((identify_primary[83] >> 10) & 1);
        log_info(supports_lba48 ? "   Supports LBA48? Yes" : "   Supports LBA48? No");
        log_number("   UDMA Modes", identify_primary[88], 2);
        uint32_t total_lba28_sectors = identify_primary[60] | (identify_primary[61] << 16);

        log_number("   Addressable Sectors", total_lba28_sectors, 10);
        log_number("   Total Bytes", total_lba28_sectors * 512, 10);
    } else {
        log_info("Secondary disk:");
        bool supports_lba48 = (bool)((identify_secondary[83] >> 10) & 1);
        log_info(supports_lba48 ? "   Supports LBA48? Yes" : "   Supports LBA48? No");
        log_number("   UDMA Modes", identify_secondary[88], 2);
        uint32_t total_lba28_sectors = identify_secondary[60] | (identify_secondary[61] << 16);

        log_number("   Addressable Sectors", total_lba28_sectors, 10);
        log_number("   Total Bytes", total_lba28_sectors * 512, 10);
    }
}

// Identify either primary (selector = 0) or secondary (selector = 1) ATA drive
void disk_identify(uint8_t selector) {
    if (selector > 1) return;
    // First we need to select the drive
    uint8_t out = ATA_MASK_DRIVE_HEAD_ALWAYSSET
                | (selector ? ATA_MASK_DRIVE_HEAD_DRV : 0); // Select drive

    outb(selector ? ATA_SECONDARY_DRIVE_HEAD : ATA_PRIMARY_DRIVE_HEAD, out);
    // Then set sector count and LBA addr regs to 0
    outb(selector ? ATA_SECONDARY_SEC_COUNT : ATA_PRIMARY_SEC_COUNT, 0x00);
    outb(selector ? ATA_SECONDARY_LBALO : ATA_PRIMARY_LBALO, 0x00);
    outb(selector ? ATA_SECONDARY_LBAMID : ATA_PRIMARY_LBAMID, 0x00);
    outb(selector ? ATA_SECONDARY_LBAHI : ATA_PRIMARY_LBAHI, 0x00);
    // Finally send IDENTIFY command
    outb(selector ? ATA_SECONDARY_STATUS_CMD : ATA_PRIMARY_STATUS_CMD, ATA_CMD_IDENTIFY);

    uint8_t status = inb(selector ? ATA_SECONDARY_STATUS_CMD : ATA_PRIMARY_STATUS_CMD);
    if (status != 0x00) {
        // Drive exists
        while (status & ATA_MASK_STATUS_BSY) { // Poll until BSY clears
            status = inb(selector ? ATA_SECONDARY_STATUS_CMD : ATA_PRIMARY_STATUS_CMD);
            uint8_t lbamid = inb(selector ? ATA_SECONDARY_LBAMID : ATA_PRIMARY_LBAMID);
            uint8_t lbahi = inb(selector ? ATA_SECONDARY_LBAHI : ATA_PRIMARY_LBAHI);
            if (lbamid || lbahi) {
                log_error("disk: err: LBAmid or LBAhi was set. primary drive not ATA.");
                return;
            }
        }

        while (!(status & ATA_MASK_STATUS_DRQ || status & ATA_MASK_STATUS_ERR)) {
            status = inb(selector ? ATA_SECONDARY_STATUS_CMD : ATA_PRIMARY_STATUS_CMD);
        }

        if (status & ATA_MASK_STATUS_DRQ) {
            // DRQ bit set, transfer 256 bytes
            for (int i = 0; i < 256; i++) {
                identify_primary[i] = inw(selector ? ATA_SECONDARY_DATA : ATA_PRIMARY_DATA);
            }
            if (selector) {
                has_secondary = TRUE;
            } else {
                has_primary = TRUE;
            }
            return;
        } else {
            // ERR bit set, abort
            log_error("disk: err: ERR bit set when attepting to identify disk.");
            return;
        }
    } else {
        // Drive does not exist
        if (selector) {
            log_info("disk: secondary drive not present.");
        } else {
            log_error("disk: err: primary drive not present.");
        }
    }

}


// Called every time an IRQ is triggered for a read operation,
// which indicates that one sector needs to be read.
void read_sector(uint16_t* buf) {
    state = READING_SECTOR;

    for(int i = 0; i < 256; i++) {
        // Read 256 16-bit values (one sector)
        buf[current_sector * 256 + i] = inw(ATA_PRIMARY_DATA);
    }

    // Check if we're done reading all sectors
    if (current_sector == sector_count - 1) {
        // If done, reset
        target_buffer = 0;
        sector_count = 0;
        current_sector = 0;
        state = IDLE;
    } else {
        // If not done, wait for next interrupt
        state = PENDING_READ;
    }
    current_sector++;
}

void read_sectors(uint32_t lba, uint8_t num_sectors, uint8_t* dest) {
    uint8_t lba_lo = lba & 0xFF;
    uint8_t lba_mid = (lba >> 8) & 0xFF;
    uint8_t lba_hi = (lba >> 16) & 0xFF;
    uint8_t lba_highest = (lba >> 24) & 0x0F; // Highest 4 bits

    // If another read or write is in progress, block until that is completed.
    while (state != IDLE) {}

    // Initialize state
    state = PENDING_READ;
    target_buffer = dest;
    sector_count = num_sectors;
    current_sector = 0;

    // First, select the primary drive and tell it to use LBA addressing
    // (also send highest 4 bits of LBA)
    uint8_t out = ATA_MASK_DRIVE_HEAD_ALWAYSSET
                | ATA_MASK_DRIVE_HEAD_LBA
                | lba_highest;
    outb(ATA_PRIMARY_DRIVE_HEAD, out);
    // Next, send sector count
    outb(ATA_PRIMARY_SEC_COUNT, num_sectors);
    // Next, send rest of LBA address
    outb(ATA_PRIMARY_LBALO, lba_lo);
    outb(ATA_PRIMARY_LBAMID, lba_mid);
    outb(ATA_PRIMARY_LBAHI, lba_hi);
    // Finally, send the READ SECTORS command and wait for the sectors to be read via IRQ.


    outb(ATA_PRIMARY_STATUS_CMD, ATA_CMD_READSECTORS);

    while(state != IDLE) {
        // Wait for disk operation to complete before returning
    }
}


void write_sector(uint16_t* buf) {
    state = WRITING_SECTOR;
    log_number("writing sector", current_sector, 10);
    for(int i = 0; i < 256; i++) {
        uint16_t data = buf[current_sector * 256 + i];
        outw(ATA_PRIMARY_DATA, data);
    }

    if (current_sector == sector_count - 1) {
        target_buffer = 0;
        sector_count = 0;
        current_sector = 0;
        state = IDLE;
    } else {
        state = PENDING_WRITE;
    }

    current_sector++;

}


void write_sectors(uint32_t lba, uint8_t num_sectors, uint8_t* src) {
    uint8_t lba_lo = lba & 0xFF;
    uint8_t lba_mid = (lba >> 8) & 0xFF;
    uint8_t lba_hi = (lba >> 16) & 0xFF;
    uint8_t lba_highest = (lba >> 24) & 0x0F; // Highest 4 bits

    // If another read or write is in progress, block until that is completed.
    while (state != IDLE) {}
    log_info("begin write");
    // Initialize state
    state = PENDING_WRITE;
    target_buffer = src;
    sector_count = num_sectors;
    current_sector = 0;

    // First, select the primary drive and tell it to use LBA addressing
    // (also send highest 4 bits of LBA)
    uint8_t out = ATA_MASK_DRIVE_HEAD_ALWAYSSET
                | ATA_MASK_DRIVE_HEAD_LBA
                | lba_highest;
    outb(ATA_PRIMARY_DRIVE_HEAD, out);
    // Next, send sector count
    outb(ATA_PRIMARY_SEC_COUNT, num_sectors);
    // Next, send rest of LBA address
    outb(ATA_PRIMARY_LBALO, lba_lo);
    outb(ATA_PRIMARY_LBAMID, lba_mid);
    outb(ATA_PRIMARY_LBAHI, lba_hi);
    // Finally, send the WRITE SECTORS command and wait for the sectors to be read via IRQ.
    outb(ATA_PRIMARY_STATUS_CMD, ATA_CMD_WRITESECTORS);

    // Start sending data.
    // For some reason the ATA controller doesn't send an IRQ immediately,
    // instead we need to give it the first sector first. After we do that
    // it starts sending IRQs.
    write_sector((uint16_t*)src);

    while(state != IDLE) {
    }
}

__attribute__((interrupt))
void disk_primary_irq(interrupt_frame* frame) {
    uint8_t status = inb(ATA_PRIMARY_STATUS_CMD);
   
    if (status & ATA_MASK_STATUS_ERR) {
        log_error("disk: err set.");

        // TODO read in error register
    }

    // Check if DRQ (data request) bit is set.
    // If set, we are in the middle of a read/write operation and should
    // read or write a sector.
    if (status & ATA_MASK_STATUS_DRQ) {
        switch (state) {
            case IDLE:
                log_error("disk IRQ recieved unexpectedly, DRQ set");
                log_number("status reg", status, 2);
                break;
            case PENDING_READ:
                read_sector((uint16_t*)target_buffer);
                break;
            case PENDING_WRITE:
                write_sector((uint16_t*)target_buffer);
                break;
            case READING_SECTOR:
                log_error("disk IRQ recieved while read in progress.");
                break;
            case WRITING_SECTOR:
                log_error("disk IRQ recieved while write in progress.");
                break;
        }
    }
    pic_eoi();
}

__attribute__((interrupt))
void disk_secondary_irq(interrupt_frame* frame) {
   log_info("Secondary disk IRQ: not implemented yet");
   pic_eoi();
}
