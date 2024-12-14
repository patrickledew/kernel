#include "fs.h"
#include "types.h"
#include "disk.h"
#include "util/logging.h"
#include "memory.h"

bios_parameter_block bpb;
fat12_pair_bytes fat[FAT_MAX_CLUSTERS / 2]; // An array of pairs of FAT clusters, 3 bytes per pair

uint16_t fat_decoded[FAT_MAX_CLUSTERS]; // An array containing each cluster in a separate index

fat_entry fat_entries[FAT_MAX_CLUSTERS]; // An array containing FAT entries, with length, beginning cluster, etc.

// Given a pointer to the start of a fat12 pair, gives the value of
// the first (which = 0) or the second (which = 1) pair.
uint16_t decode_fat12_pair(fat12_pair_bytes bytes, int which) {
    if (which > 1) return 0;

    uint16_t first = (bytes.first & 0xFF)                // Low 8 bits are first byte
                   | (bytes.second & 0x0F) << 8; // High 4 bits are low nibble of second byte

    uint16_t second = (bytes.second & 0xF0) >> 4        // Low 4 bits are high nibble of second byte
                    | (bytes.third & 0xFF) << 4;        // High 8 bits are third byte


    return which ? second : first;
}

void fs_init() {
    read_sectors(FS_START_SECTOR, 1, (uint8_t*)&bpb);
    read_sectors(FS_START_SECTOR + 1, FAT_SECTORS, (uint8_t*)fat);
    
    log_number("sectors_per_cluster", bpb.sectors_per_cluster, 16);
    log_number("num_reserved_sectors", bpb.num_reserved_sectors, 16);
    log_number("num_fats", bpb.num_fats, 16);
    log_number("num_root_directory_entries", bpb.num_root_directory_entries, 16);
    log_number("sector_count", bpb.sector_count, 16);
    log_number("media_descriptor_type", bpb.media_descriptor_type, 16);
    log_number("sectors_per_fat", bpb.sectors_per_fat, 16);
    log_number("sectors_per_track", bpb.sectors_per_track, 16);
    log_number("num_heads", bpb.num_heads, 16);
    log_number("num_hidden_sectors", bpb.num_hidden_sectors, 16);
    log_number("large_sector_count", bpb.large_sector_count, 16);

    parse_fat();

    directory_table_desc rootdir = read_directory(0);
    log_number("rootdir file count", rootdir.count, 10);
    for (int i = 0; i < rootdir.count; i++) {
        directory_entry file = rootdir.entries[i];
        // Make filename null-terminated
        print_count(file.filename, 8);
        print_char('.');
        print_count(file.extension, 3);
        println(":");

        log_number("    size", file.file_size, 10);
        log_number("    start cluster", file.start_cluster, 10);
        println("");

    }
}

void parse_fat() {
    fat12_pair_bytes* pair = fat;

    for (int i = 0; i < FAT_MAX_CLUSTERS; i += 2) {
        uint16_t first = decode_fat12_pair(*pair, 0) & 0xFFF;
        uint16_t second = decode_fat12_pair(*pair, 1) & 0xFFF;

        fat_decoded[i] = first;
        fat_decoded[i + 1] = second;
        pair++;
    }

    for (int i = 2; i < FAT_MAX_CLUSTERS; i++) {
        fat_entry entry;
        entry.begin_cluster = i;
        if (!(entry.num_clusters = file_cluster_count(i))) return;
        entry.end_cluster = entry.begin_cluster + entry.num_clusters - 1;

        fat_entries[i] = entry;
    }
}

// Traverses a FAT chain and returns the number of clusters that chain contains.
int file_cluster_count(uint16_t start_cluster) {
    if (start_cluster < 2) return 0;
    
    uint16_t num_clusters = 0;
    uint16_t i = start_cluster;
    uint16_t cluster;

    for (int i = start_cluster; i < FAT_MAX_CLUSTERS; i = cluster) {
        cluster = fat_decoded[i];

        if (cluster == FAT_CLUSTER_FREE || cluster == FAT_CLUSTER_FREE || cluster == FAT_CLUSTER_RESERVED || cluster == FAT_CLUSTER_BAD) return 0;
        // End of cluster marker. This counts as one of the clusters in the file.
        if (cluster >= FAT_CLUSTER_EOC_LO && cluster <= FAT_CLUSTER_EOC_HI) return num_clusters + 1;

        num_clusters++;
    }
}

// Reads a directory located at [start_cluster] and loads it into an array of directory_entry structs.
// If 0 is provided as the start cluster, will read the root directory.
directory_table_desc read_directory(int start_cluster) {
    uint8_t* buffer = alloc(MAX_ENTRIES_PER_DIRECTORY * 32); // Allocate enough to hold the max number of files we desire

    uint32_t start_sector;
    if (start_cluster == 0) {
        start_sector = ROOT_DIRECTORY_START_SECTOR(bpb);
    } else {
        start_sector = CLUSTER_SECTOR(start_cluster, bpb);
    }

    directory_table_desc desc;
    desc.entries = (directory_entry*)buffer;
    desc.count = 0;
    
    read_sectors(start_sector, MAX_ENTRIES_PER_DIRECTORY / DIRECTORY_ENTRIES_PER_SECTOR, buffer);
    
    // Scan buffer for the ending directory entry
    for (int i = 0; i < MAX_ENTRIES_PER_DIRECTORY; i++) {
        directory_entry entry = ((directory_entry*)buffer)[i];
        if (entry.filename[0] == 0x00) {
            desc.count = i;
            free_excess(buffer + i * sizeof(directory_entry)); // Free unused allocated space
            break;
        }
    }
    return desc;
}