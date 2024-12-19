#ifndef FAT12_H
#define FAT12_H

#include "types.h"

// Filesystem code
// For now i'm going to be attempting to use a FAT12 filesystem,
// the partion for which is located immediately after the kernel image.

#define FS_START 0x10200 // Filesystem starts in next sector after kernel image
#define FS_END 0x110600
#define FS_START_SECTOR (FS_START / 512)
#define FS_END_SECTOR (FS_END / 512)

// Hardcoded for now, is there a way to get this information?
#define FAT_BYTES 0x400
#define FAT_SECTORS (FAT_BYTES / 512)
#define FAT_MAX_CLUSTERS (FAT_BYTES / 3 * 2)
#define FAT_DATA_START 0x5200

#define FAT_CLUSTER_FREE        0x000
#define FAT_CLUSTER_RESERVED    0x001
#define FAT_CLUSTER_BAD         0xFF7
#define FAT_CLUSTER_EOC_LO      0xFF8 // End of chain markers can be in the range 0xFF8-0xFFF
#define FAT_CLUSTER_EOC_HI      0xFFF
#define FAT_SECTORS_PER_CLUSTER

#define FAT_ATTRIBUTE_MASK_READONLY 0x01 // Read only
#define FAT_ATTRIBUTE_MASK_HIDDEN 0x02 // Hidden
#define FAT_ATTRIBUTE_MASK_SYSTEM 0x04 // System
#define FAT_ATTRIBUTE_MASK_VOLLABEL 0x08 // Volume Label
#define FAT_ATTRIBUTE_MASK_SUBDIR 0x10 // Subdirectory
#define FAT_ATTRIBUTE_MASK_ARCHIVE 0x20 // Archive
#define FAT_ATTRIBUTE_MASK_DEVICE 0x40 // Device
#define FAT_ATTRIBUTE_MASK_RESERVED 0x80 // Reserved

#define MAX_ENTRIES_PER_DIRECTORY 0x200 // Use until we implement dynamic allocation for directory tables
#define DIRECTORY_ENTRY_SIZE 32
#define DIRECTORY_ENTRIES_PER_SECTOR (0x200 / DIRECTORY_ENTRY_SIZE) // 32 byte directory entries

// Starting sector of the data region.
#define ROOT_DIRECTORY_START_SECTOR(bpb) (FS_START_SECTOR + bpb.num_reserved_sectors + (bpb.num_fats * bpb.sectors_per_fat))
#define DATA_REGION_START_SECTOR(bpb) (ROOT_DIRECTORY_START_SECTOR(bpb) + (bpb.num_root_directory_entries / DIRECTORY_ENTRIES_PER_SECTOR))

// Starting sector of a given cluster.
// Clusters are relative to the data region, with the first sector (offset 0x0) being cluster 2.
#define CLUSTER_SECTOR(cluster_number, bpb) (DATA_REGION_START_SECTOR(bpb) + (cluster_number - 2) * bpb.sectors_per_cluster)

typedef struct {
    uint8_t first;
    uint8_t second;
    uint8_t third;
} __attribute__((__packed__)) Fat12Pair;

typedef struct {
    char ___jmpins[3];
    char oem[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t num_reserved_sectors;
    uint8_t num_fats;
    uint16_t num_root_directory_entries;
    uint16_t sector_count;
    uint8_t media_descriptor_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t num_hidden_sectors;
    uint32_t large_sector_count;
} __attribute__((__packed__)) BiosParameterBlock;

typedef struct {
    uint16_t begin_cluster;
    uint16_t num_clusters;
} FATEntry;

typedef struct {
    char filename[8];
    char extension[3];
    uint8_t attributes; // See FAT_ATTRIBUTE_MASK_ defines
    uint8_t __unused[10]; // Other OS-specific things or not relevant to FAT12
    uint16_t last_modified_time;
    uint16_t last_modified_date;
    uint16_t start_cluster;
    uint32_t file_size; // in bytes
} __attribute__((__packed__)) FATFile;

typedef struct {
    FATFile* entries;
    uint16_t count;
} FATDirectoryTable;


void fat_init();
uint16_t fat_pair_decode(Fat12Pair bytes, uint8_t which);
void fat_read();
int fat_file_num_clusters(uint16_t start_cluster);
FATDirectoryTable fat_directory_read(uint16_t start_cluster);

#endif