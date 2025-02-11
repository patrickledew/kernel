#include "disk.h"
#include "core/portio/portio.h"
#include "util/logging.h"
#include "core/mem/alloc.h"
#include "util/assert.h"
// This file contains an implementation for interfacing with ATA hard drives directly, through port IO.
// Note: this method is inherently slow and there exist other (much faster) solutions, namely DMA.

// Another, very crucial TODO: This approach DOES NOT consider multitasking whatsoever.
// Currently it's just a state machine where we only expect one read to be performed at
// a time, and we just block until any ongoing reads or writes are completed.
// 
// Really, we should be using some sort of queue, and performing other tasks while we wait
// instead of simply blocking.

uint16_t disk_info[256] = {0};

uint8_t* target_buffer = 0x0000; // The buffer we're writing disk data to (when reading from disk) or reading from (when writing to disk)
uint8_t sector_count = 0; // Number of sectors we are currently reading/writing.
uint8_t current_sector = 0; // Current sector we are reading/writing.

uint32_t last_lba = 0;
DiskState state = IDLE;

void _assert_valid_sector(uint32_t lba, int line, char* file) {
    uint32_t total_lba28_sectors = disk_info[60] | (disk_info[61] << 16);
    _assert_lt_u32(total_lba28_sectors, lba, line, file);
}
#define assert_valid_sector(lba) _assert_valid_sector(lba, __LINE__, __FILE__);

bool disk_detect_floating() {
    uint8_t in = inb(ATA_PRIMARY_STATUS_CMD); // Read status byte
    return in == 0xFF;
}

void disk_init() {
    log_info("disk_init: initializing ATA disk driver.");
    REG_ISR(ATA_PRIMARY_IRQ, disk_irq);

    if (disk_detect_floating()) {
        log_error("disk_init: err: no drives connected.");
        return;
    }

    disk_identify();

    disk_log_info();
}

void disk_log_info() {
    log_info("Disk Info:");
    bool supports_lba48 = (bool)((disk_info[83] >> 10) & 1);
    log_info(supports_lba48 ? "   Supports LBA48? Yes" : "   Supports LBA48? No");
    log_number_u("   UDMA Modes", disk_info[88], 2);
    uint32_t total_lba28_sectors = disk_info[60] | (disk_info[61] << 16);

    log_number_u("   Addressable Sectors", total_lba28_sectors, 10);
    log_number_u("   Total Bytes", total_lba28_sectors * 512, 10);
}

// Identify either primary (selector = 0) or secondary (selector = 1) ATA drive
void disk_identify() {
    // First we need to select the drive
    outb(ATA_PRIMARY_DRIVE_HEAD, ATA_MASK_DRIVE_HEAD_ALWAYSSET);
    // Then set sector count and LBA addr regs to 0
    outb(ATA_PRIMARY_SEC_COUNT, 0x00);
    outb(ATA_PRIMARY_LBALO, 0x00);
    outb(ATA_PRIMARY_LBAMID, 0x00);
    outb(ATA_PRIMARY_LBAHI, 0x00);
    // Finally send IDENTIFY command
    outb(ATA_PRIMARY_STATUS_CMD, ATA_CMD_IDENTIFY);

    uint8_t status = inb(ATA_PRIMARY_ALTSTATUS_CONTROL);
    if (status != 0x00) {
        // Drive exists
        while (status & ATA_MASK_STATUS_BSY) { // Poll until BSY clears
            status = inb(ATA_PRIMARY_ALTSTATUS_CONTROL);
            uint8_t lbamid = inb(ATA_PRIMARY_LBAMID);
            uint8_t lbahi = inb(ATA_PRIMARY_LBAHI);
            if (lbamid || lbahi) {
                log_error("disk: err: LBAmid or LBAhi was set. primary drive not ATA.");
                return;
            }
        }

        while (!(status & ATA_MASK_STATUS_DRQ || status & ATA_MASK_STATUS_ERR)) {
            status = inb(ATA_PRIMARY_ALTSTATUS_CONTROL);
        }

        if (status & ATA_MASK_STATUS_DRQ) {
            // DRQ bit set, transfer 256 bytes
            for (int i = 0; i < 256; i++) {
                disk_info[i] = inw(ATA_PRIMARY_DATA);
            }
            return;
        } else {
            // ERR bit set, abort
            log_error("disk: err: ERR bit set when attepting to identify disk.");
            return;
        }
    } else {
        // Drive does not exist
        log_error("disk: err: drive not present.");
    }

}


// Called every time an IRQ is triggered for a read operation,
// which indicates that one sector needs to be read.
void disk_sector_read(uint16_t* buf) {
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
        current_sector++;
        state = PENDING_READ;
    }
}

void disk_read(uint32_t lba, uint8_t num_sectors, uint8_t* dest) {
    assert_valid_sector(lba);
    last_lba = lba;
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

void disk_sector_write(uint16_t* buf) {
    state = WRITING_SECTOR;
    for(int i = 0; i < 256; i++) {
        uint16_t data = buf[current_sector * 256 + i];
        outw(ATA_PRIMARY_DATA, data);
    }

    if (current_sector >= sector_count - 1) {
        target_buffer = 0;
        sector_count = 0;
        current_sector = 0;
        state = FINISHING_WRITE;
    } else {
        current_sector++;
        state = PENDING_WRITE;
    }
}


void disk_write(uint32_t lba, uint8_t num_sectors, uint8_t* src) {
    assert_valid_sector(lba);
    last_lba = lba;

    uint8_t lba_lo = lba & 0xFF;
    uint8_t lba_mid = (lba >> 8) & 0xFF;
    uint8_t lba_hi = (lba >> 16) & 0xFF;
    uint8_t lba_highest = (lba >> 24) & 0x0F; // Highest 4 bits

    // If another read or write is in progress, block until that is completed.
    while (state != IDLE) {}

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
    disk_sector_write((uint16_t*)src);

    while(state != IDLE) {
    }
}

__attribute__((interrupt))
void disk_irq(InterruptFrame* frame) {
    uint8_t status = inb(ATA_PRIMARY_STATUS_CMD);
   
    if (status & ATA_MASK_STATUS_ERR) {
        log_error("disk: err set.");
        uint8_t error = inb(ATA_PRIMARY_ERROR_FEATURES);
        if (error & ATA_MASK_ERR_AMNF) log_error("disk: err: Address mark not found.");
        if (error & ATA_MASK_ERR_TKZNF) log_error("disk: err: Track zero not found. ");
        if (error & ATA_MASK_ERR_ABRT) log_error("disk: err: Aborted command. ");
        if (error & ATA_MASK_ERR_MCR) log_error("disk: err: Media change request. ");
        if (error & ATA_MASK_ERR_IDNF) log_error("disk: err: ID not found. ");
        if (error & ATA_MASK_ERR_MC) log_error("disk: err: Media changed. ");
        if (error & ATA_MASK_ERR_UNC) log_error("disk: err: Uncorrectable data error. ");
        if (error & ATA_MASK_ERR_BBK) log_error("disk: err: Bad Block detected.");
        log_number_u("  Starting Sector", last_lba, 16);
        log_number_u("  Sector Offset", current_sector, 16);
        log_number_u("  # Sectors", sector_count, 16);
        uint32_t total_lba28_sectors = disk_info[60] | (disk_info[61] << 16);
        log_number_u("  Total Addressable Sectors", total_lba28_sectors, 16);
    }


    // Check if DRQ (data request) bit is set.
    // If set, we are in the middle of a read/write operation and should
    // read or write a sector.
    if (status & ATA_MASK_STATUS_DRQ) {
        switch (state) {
            case IDLE:
                log_error("disk IRQ recieved unexpectedly, DRQ set");
                break;
            case PENDING_READ:
                disk_sector_read((uint16_t*)target_buffer);
                break;
            case PENDING_WRITE:
                disk_sector_write((uint16_t*)target_buffer);
                break;
            case READING_SECTOR:
                log_info("READING_SECTOR");
                log_error("disk: IRQ with DRQ set while already reading data.");
                break;
            case WRITING_SECTOR:
                log_info("WRITING_SECTOR");
                log_error("disk: IRQ with DRQ set while already writing data.");
                break;
            case FINISHING_WRITE:
                log_error("disk: IRQ with DRQ set after write complete.");
        }
    } else {
        if (state == FINISHING_WRITE) {
            state = IDLE; // For whatever reason, write operations issue an additional interrupt that must be consumed before allowing another write.
        }
    }
    int_pic_send_eoi();
}