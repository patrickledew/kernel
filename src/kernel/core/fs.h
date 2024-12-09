#ifndef FS_H
#define FS_H

#include "types.h"

// Filesystem code
// For now i'm going to be attempting to use a FAT12 filesystem,
// the partion for which is located immediately after the kernel image.

#define FS_START 0x10200 // Filesystem starts in next sector after kernel image

typedef struct {
    uint8_t byte1;
    uint8_t byte2;
    uint8_t byte3;
} __attribute__((packed)) fat12_entry;

fat12_entry uint16_to_fat12(uint16_t in);
uint16_t fat12_to_uint16(fat12_entry in);


#endif