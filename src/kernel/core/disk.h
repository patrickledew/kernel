#ifndef DISK_H
#define DISK_H
#include "types.h"
#include "interrupts.h"
// This file contains defines for interfacing with ATA hard drives directly, through port IO.
// Note: this method is inherently slow and there exist other (much faster) solutions, namely DMA.

#define ATA_PRIMARY_IRQ   0x2E
#define ATA_SECONDARY_IRQ   0x2F

#define ATA_PRIMARY_DATA            0x1F0 // Data Register (R/W)
#define ATA_PRIMARY_ERROR_FEATURES  0x1F1 // Error Register (R) and Features Register (W)
#define ATA_PRIMARY_SEC_COUNT       0x1F2 // Sector Count Register (R/W)
#define ATA_PRIMARY_LBALO           0x1F3 // Sector Number / LBALo Register (R/W)
#define ATA_PRIMARY_LBAMID          0x1F4 // Cylinder Low / LBAMid Register (R/W)
#define ATA_PRIMARY_LBAHI           0x1F5 // Cylinder High / LBAHi Register (R/W)
#define ATA_PRIMARY_DRIVE_HEAD      0x1F6 // Drive Select / Head Register (R/W)
#define ATA_PRIMARY_STATUS_CMD      0x1F7 // Status Register (R) or Command Register (W)

#define ATA_PRIMARY_ALTSTATUS_CONTROL   0x3F6 // Alternate Status (R) or Device Control Register (W)
#define ATA_PRIMARY_DRIVE_ADDR          0x3F7 // Drive Address Register (R)

#define ATA_SECONDARY_DATA            0x170 // Data Register (R/W)
#define ATA_SECONDARY_ERROR_FEATURES  0x171 // Error Register (R) and Features Register (W)
#define ATA_SECONDARY_SEC_COUNT       0x172 // Sector Count Register (R/W)
#define ATA_SECONDARY_LBALO           0x173 // Sector Number / LBALo Register (R/W)r Register (R/W)
#define ATA_SECONDARY_LBAMID          0x174 // Cylinder Low / LBAMid Register (R/W)
#define ATA_SECONDARY_LBAHI           0x175 // Cylinder High / LBAHi Register (R/W)
#define ATA_SECONDARY_DRIVE_HEAD      0x176 // Drive Select / Head Register (R/W)
#define ATA_SECONDARY_STATUS_CMD      0x177 // Status Register (R) or Command Register (W)

#define ATA_SECONDARY_ALTSTATUS_CONTROL   0x376 // Alternate Status (R) or Device Control Register (W)
#define ATA_SECONDARY_DRIVE_ADDR          0x377 // Drive Address Register (R)

#define ATA_MASK_ERR_AMNF   0b1         // Address mark not found
#define ATA_MASK_ERR_TKZNF  0b10        // Track zero not found
#define ATA_MASK_ERR_ABRT   0b100       // Aborted command
#define ATA_MASK_ERR_MCR    0b1000      // Media change request
#define ATA_MASK_ERR_IDNF   0b10000     // ID not found
#define ATA_MASK_ERR_MC     0b100000    // Media Changed
#define ATA_MASK_ERR_UNC    0b1000000   // Uncorrectable data error
#define ATA_MASK_ERR_BBK    0b10000000  // Bad block detected

#define ATA_MASK_DRIVE_HEAD_LOW           0b1111 // Bits 0-3 of head, or 24-27 of LBA address
#define ATA_MASK_DRIVE_HEAD_DRV          0b10000 // Selects drive number (primary or secondary)
#define ATA_MASK_DRIVE_HEAD_LBA        0b1000000 // Use CHS addressing if clear, LBA if set
#define ATA_MASK_DRIVE_HEAD_ALWAYSSET 0b10100000 // Bits that should always be set

#define ATA_MASK_STATUS_ERR        0b1 // Indicates error has occurred
#define ATA_MASK_STATUS_DRQ     0b1000 // Set when drive has PIO data to transfer, or is ready to accept PIO data
#define ATA_MASK_STATUS_SRV    0b10000 // Overlapped mode service request
#define ATA_MASK_STATUS_DF    0b100000 // Drive fault error
#define ATA_MASK_STATUS_RDY  0b1000000 // Bit clear when drive spun down or after error. Set otherwise.
#define ATA_MASK_STATUS_BSY 0b10000000 // Set when drive preparing to send/receive data

#define ATA_MASK_DEVCTRL_NEIN   0b10        // Set to DISABLE interrupts from current device
#define ATA_MASK_DEVCTRL_SRST   0b100       // Set then clear to perform a software reset
#define ATA_MASK_DEVCTRL_HOB    0b10000000  // Set to read back High Order Byte of last LBA48 value sent to IO port

#define ATA_MASK_DRIVE_ADDR_DS0 0b1         // Drive 0 select (0 is selected)
#define ATA_MASK_DRIVE_ADDR_DS1 0b10        // Drive 1 select (0 is selected)
#define ATA_MASK_DRIVE_ADDR_HS  0b111100    // One's complement of currently selected head
#define ATA_MASK_DRIVE_ADDR_WTG 0b1000000   // Write gate, 0 when write in progress

#define ATA_CMD_IDENTIFY 0xEC
#define ATA_CMD_READSECTORS 0x20
#define ATA_CMD_WRITESECTORS 0x30



typedef enum {
    IDLE, PENDING_READ, PENDING_WRITE, READING_SECTOR, WRITING_SECTOR
} disk_state;
// Detect floating bus if no drive is connected
bool disk_detect_floating();

void disk_init();
void disk_identify(uint8_t selector);

void log_disk_info();

void disk_primary_irq(interrupt_frame* frame);
void disk_secondary_irq(interrupt_frame* frame);

void read_sector(uint16_t* buf);
void read_sectors(uint32_t lba, uint8_t num_sectors, uint8_t* dest);

void write_sector(uint16_t* buf);
void write_sectors(uint32_t lba, uint8_t num_sectors, uint8_t* src);

#endif
