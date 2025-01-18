#include "fat12.h"
#include "types.h"
#include "core/disk/disk.h"
#include "util/logging.h"
#include "core/mem/alloc.h"
#include "core/mem/memory.h"
#include "util/assert.h"
BiosParameterBlock bpb;
Fat12Pair fat_raw[FAT_MAX_CLUSTERS / 2]; // An array of pairs of FAT clusters, 3 bytes per pair

uint16_t fat[FAT_MAX_CLUSTERS]; // An array containing each cluster in a separate index

FATEntry fat_entries[FAT_MAX_CLUSTERS]; // An array containing FAT entries, with length, beginning cluster, etc.

FATDirectory _read_and_print_dir(uint32_t cluster) {
    FATDirectory directory = fat_directory_read(0);
    log_number_u("rootdir file count", directory.count, 10);
    for (int i = 0; i < directory.count; i++) {
        FATFile file = directory.entries[i];
        print_count(file.filename, 8);
        print_char('.');
        print_count(file.extension, 3);
        print(": [");
        if (file.attributes & FAT_ATTRIBUTE_MASK_SUBDIR) {
            print("Directory, ");
        }
        print_num_u(file.file_size, 10);
        print(" Bytes, Cluster ");
        print_num_u(file.start_cluster, 10);
        println("]");
    }
    return directory;
}

void fat_init() {
    disk_read(FS_START_SECTOR, 1, (uint8_t*)&bpb);
    disk_read(FS_START_SECTOR + 1, FAT_SECTORS, (uint8_t*)fat_raw);
    
    fat_read();
}

// Given a pointer to the start of a fat12 pair, gives the value of
// the first (which = 0) or the second (which = 1) pair.
uint16_t fat_pair_decode(Fat12Pair bytes, uint8_t which) {
    if (which > 1) return 0;

    uint16_t first = (bytes.first & 0xFF)                // Low 8 bits are first byte
                   | (bytes.second & 0x0F) << 8; // High 4 bits are low nibble of second byte

    uint16_t second = (bytes.second & 0xF0) >> 4        // Low 4 bits are high nibble of second byte
                    | (bytes.third & 0xFF) << 4;        // High 8 bits are third byte


    return which ? second : first;
}

Fat12Pair fat_pair_encode(uint16_t first, uint16_t second) {
    Fat12Pair pair;
    pair.first = first & 0xFF;
    pair.second = ((first >> 8) & 0x0F) | ((second << 4) & 0xF0);
    pair.third = (second >> 4) & 0xFF;
    return pair;
}

void fat_read() {
    Fat12Pair* pair = fat_raw;

    for (int i = 0; i < FAT_MAX_CLUSTERS; i += 2) {
        uint16_t first = fat_pair_decode(*pair, 0) & 0xFFF;
        uint16_t second = fat_pair_decode(*pair, 1) & 0xFFF;

        fat[i] = first;
        fat[i + 1] = second;
        pair++;
    }

    for (int i = 2; i < FAT_MAX_CLUSTERS; i++) {
        FATEntry entry;
        entry.begin_cluster = i;
        if (!(entry.num_clusters = fat_file_num_clusters(i))) return;
        
        fat_entries[i] = entry;
    }
}

void fat_flush() {
    // First, encode into fat_raw
    Fat12Pair* pair = fat_raw;
    for (int i = 0; i < FAT_MAX_CLUSTERS; i += 2) {
        uint16_t first = fat[i];
        uint16_t second = fat[i+1];
        // Need to squeeze these into 3 bytes in the actual FAT
        *pair = fat_pair_encode(first, second);
        pair++;
    }

    // Next, write that to disk
    for (int i = 0; i < bpb.num_fats; i++) {
        disk_write(FS_START_SECTOR + (i * FAT_SECTORS) +1, FAT_SECTORS, (uint8_t*)fat_raw);
    }
}

// Traverses a FAT chain and returns the number of clusters that chain contains.
int fat_file_num_clusters(uint16_t start_cluster) {
    if (start_cluster < 2) return 0;
    
    uint16_t num_clusters = 0;
    uint16_t i = start_cluster;
    uint16_t cluster;

    for (int i = start_cluster; i < FAT_MAX_CLUSTERS; i = cluster) {
        cluster = fat[i];

        if (cluster == FAT_CLUSTER_FREE || cluster == FAT_CLUSTER_FREE || cluster == FAT_CLUSTER_RESERVED || cluster == FAT_CLUSTER_BAD) return 0;
        // End of cluster marker. This counts as one of the clusters in the file.
        if (cluster >= FAT_CLUSTER_EOC_LO && cluster <= FAT_CLUSTER_EOC_HI) return num_clusters + 1;

        num_clusters++;
    }
}

// Reads a directory located at [start_cluster] and loads it into FATDirectory structure.
// If 0 is provided as the start cluster, will read the root directory.
// To avoid memory leaks, call free(result.entries) as this is dynamically allocated.
FATDirectory fat_directory_read(uint16_t start_cluster) {
    FATDirectory dir;
    uint32_t bytes_per_cluster = bpb.bytes_per_sector * bpb.sectors_per_cluster;
    
    uint8_t* buffer = alloc(DIRECTORY_ENTRY_SIZE * MAX_ENTRIES_PER_DIRECTORY); // Allocate enough to hold the max number of files we desire

    uint32_t cluster = start_cluster; // The cluster we are currently reading. If reading root directory, functions as offset (in clusters) from beginning of root dir.
    uint32_t clusters_read = 0; // The total clusters we've read so far.

    dir.entries = (FATFile*)buffer;
    dir.count = 0;
    dir.start_cluster = start_cluster;

    bool done = FALSE;
    do {
        // fill buffer with current cluster 
        uint8_t* dest = buffer + clusters_read * bytes_per_cluster;
        if (start_cluster == 0) {
            uint32_t sector = ROOT_DIRECTORY_START_SECTOR(bpb) + cluster * bpb.sectors_per_cluster;
            disk_read(sector, bpb.sectors_per_cluster, dest);
        } else {
            disk_read(CLUSTER_SECTOR(cluster, bpb), bpb.sectors_per_cluster, dest);
        }

        // traverse buffer to find end entry
        for (int i = 0; i < DIRECTORY_ENTRIES_PER_SECTOR * bpb.sectors_per_cluster; i++) {
            FATFile entry = ((FATFile*)dest)[i];
            if (entry.filename[0] == 0x00) {
                done = TRUE;
                break;
            } else {
                dir.count++;
            }
        }
        // If reading root directory, increment cluster to get next
        if (start_cluster == 0) {
            cluster++;
        } else if (fat[cluster] < FAT_CLUSTER_EOC_LO) {
            cluster = fat[cluster];
        } else if (!done) {
            log_error("fat: could not find terminating directory entry.");
            done = TRUE;
        }

        clusters_read++;
    } while (!done);


    // Can't think of an easy way to grow/shrink this buffer without a better memory allocator.
    // For now we'll just allocate a stupid amount of memory (max files allowed) and deal with it.
    // free_excess(buffer + (dir.count + 1) * sizeof(FATFile)); // Free unused allocated space
    return dir;
}

int fat_directory_write(FATDirectory* directory) {
    uint32_t bytes_per_cluster = bpb.bytes_per_sector * bpb.sectors_per_cluster;
    
    uint8_t* cluster_buf = alloc(bytes_per_cluster); // Allocate enough to hold the max number of files we desire
    uint32_t cluster = directory->start_cluster; // The cluster we are currently reading. If reading root directory, functions as offset (in clusters) from beginning of root dir.
    uint32_t clusters_written = 0; // The total clusters we've read so far.

    bool done = FALSE;
    do {
        uint32_t entries_written = clusters_written * DIRECTORY_ENTRIES_PER_SECTOR * bpb.sectors_per_cluster;
        FATFile* src = directory->entries + entries_written;
        // initialize cluster_buf to 0s
        memfill(cluster_buf, bytes_per_cluster, 0);
        // write directory entries one at a time into cluster buf (including 0 entry)
        for (int i = 0; i < DIRECTORY_ENTRIES_PER_SECTOR * bpb.sectors_per_cluster; i++) {
            FATFile entry = src[i];
            ((FATFile*)cluster_buf)[i] = entry;

            if (entry.filename[0] == 0x00 || directory->count < entries_written + i) {
                done = TRUE;
                break;
            }
        }

        // write cluster_buf to disk according to cluster
        if (directory->start_cluster == 0) {
            disk_write(ROOT_DIRECTORY_START_SECTOR(bpb) + cluster * bpb.sectors_per_cluster, bpb.sectors_per_cluster, cluster_buf);
        } else {
            disk_write(CLUSTER_SECTOR(cluster, bpb), bpb.sectors_per_cluster, cluster_buf);
        }

        // If writing root directory, increment cluster to get next
        if (directory->start_cluster == 0) {
            cluster++;
        } else if (fat[cluster] < FAT_CLUSTER_EOC_LO) {
            cluster = fat[cluster];
        } else if (!done) {
            // Need to find new cluster
            cluster = fat_chain_add(cluster);
            if (cluster == 0) {
                return -1;
            }
        }

        clusters_written++;
    } while(!done);

    return 0;
}

// problem: what this is called with a `skip` at the start of an unallocated cluster (e.g., when appending a file)
// it will return the EOC marker rather than a valid cluster.

// Traverse FAT and get cluster number that is `skip` bytes into the file.
uint16_t __get_cluster_from_offset(FATFile* file, uint32_t skip) {
    if (skip > file->file_size) return 0;

    uint32_t bytes_per_cluster = bpb.bytes_per_sector * bpb.sectors_per_cluster;

    uint16_t cluster = file->start_cluster;
    for (int c = 0; c < skip / bytes_per_cluster && cluster < FAT_CLUSTER_EOC_LO; c++) // c = distance in clusters of the current cluster from beginning of file
    {
        cluster = fat[cluster];
    }

    if (cluster >= FAT_CLUSTER_EOC_LO) return 0;
    
    return cluster;
}

uint16_t __get_last_cluster(FATFile* file) {
    uint16_t cluster = file->start_cluster;
    for (int c = 0; c < FAT_MAX_CLUSTERS; c++) {
        if (fat[cluster] < FAT_CLUSTER_EOC_LO) {
            cluster = fat[cluster];
        } else {
            return cluster;
        }
    }
    return 0;
}

int fat_file_read(FATFile* file, uint8_t* buf, uint32_t length, uint32_t skip) {
    if (length == 0) return -1;
    if (skip >= file->file_size) return -1;

    uint32_t bytes_per_cluster = bpb.bytes_per_sector * bpb.sectors_per_cluster;
    uint32_t position = skip;
    uint16_t cluster = __get_cluster_from_offset(file, skip);
    if (cluster == 0) {
        log_error("fat: err: could not find cluster for given position.");
        return -1;
    }

    uint8_t* cluster_buf = alloc(bytes_per_cluster);
    
    while (1) {
        // We shouldn't be trying to read cluster if it's an EOC marker
        // In fact, this shouldn't even be possible.
        assert_lt_u16(FAT_CLUSTER_EOC_LO, cluster);

        uint32_t sector_number = CLUSTER_SECTOR(cluster, bpb);

        assert_lt_u32(bpb.sector_count, sector_number); // Sector number should be less than total sectors

        disk_read(sector_number, bpb.sectors_per_cluster, cluster_buf);
        do {
            buf[position - skip] = cluster_buf[position % bytes_per_cluster];
            position++;
        } while(position % bytes_per_cluster != 0
             && position < length + skip
             && position < file->file_size);

        // If current position is the file's size or we've read `length` bytes,
        // we're done.
        if (position == file -> file_size || position == length + skip) break;
        // If next FAT entry is end of chain, we've reached the end of the file.
        if (fat[cluster] == FAT_CLUSTER_EOC_HI || fat[cluster] == FAT_CLUSTER_EOC_LO) break;

        // Otherwise, we keep reading clusters
        cluster = fat[cluster];
    }
    free(cluster_buf);
    return position - skip;
}

uint32_t __cluster_rewrite(uint16_t cluster, uint8_t* cluster_buf, uint8_t* buf, uint32_t position, uint32_t length, uint32_t skip) {
    uint32_t bytes_per_cluster = bpb.bytes_per_sector * bpb.sectors_per_cluster;
    // Write `cluster` by reading in existing cluster and overwriting bytes.
    // It will start reading buf starting at `position - skip`, and write those bytes
    // to the disk starting at `position` bytes into the file.
    // until it either reaches the end of the cluster, or the end of the buffer
    // (length is the total size of the buffer)
    disk_read(CLUSTER_SECTOR(cluster, bpb), bpb.sectors_per_cluster, cluster_buf);
    // cluster_buf now has the existing cluster
    // position - skip is offset from start of buf
    do {
            cluster_buf[position % bytes_per_cluster] = buf[position - skip];
            position++;
    } while (position % bytes_per_cluster != 0 && (position - skip) < length);
    // cluster_buf now overwritten with correct data
    disk_write(CLUSTER_SECTOR(cluster, bpb), bpb.sectors_per_cluster, cluster_buf);
    
    // If we reached the end of the buffer, return 0 to indicate we're done writing
    if ((position - skip) == length) return 0;

    // Otherwise return the current position in the file so we can update it
    return position;
}

uint32_t __cluster_writenew(uint16_t cluster, uint8_t* cluster_buf, uint8_t* buf, uint32_t position, uint32_t length, uint32_t skip) {
    uint32_t bytes_per_cluster = bpb.bytes_per_sector * bpb.sectors_per_cluster;

    memfill(cluster_buf, bytes_per_cluster, 0x00);
    // cluster_buf is filled with 0s now
    // todo use memcpy maybe?
    do {
            cluster_buf[position % bytes_per_cluster] = buf[position - skip];
            position++;
    } while (position % bytes_per_cluster != 0 && (position - skip) < length);
    // cluster_buf now overwritten with correct data
    disk_write(CLUSTER_SECTOR(cluster, bpb), bpb.sectors_per_cluster, cluster_buf);

    // If we reached the end of the buffer, return -1 to indicate we're done writing
    if ((position - skip) == length) return 0;

    return position; // Otherwise, return position so we can update it
}

uint16_t fat_chain_add(uint16_t cluster) {
    // First, find an unreserved FAT entry
    for (int i = cluster; i < FAT_MAX_CLUSTERS; i++) {
        if (fat[i] == 0x00) { // If cluster is free
            fat[cluster] = i;
            fat[i] = FAT_CLUSTER_EOC_HI; // Reserve it
            fat_flush();
            return i;
        }
    }
    return 0; // If we couldn't find an available cluster, return 0
}

uint16_t fat_chain_start() {
    // First, find an unreserved FAT entry
    for (int i = 3; i < FAT_MAX_CLUSTERS; i++) {
        if (fat[i] == 0x00) { // If cluster is free
            fat[i] = FAT_CLUSTER_EOC_HI; // Reserve it
            fat_flush();
            return i;
        }
    }
    return 0; // If we couldn't find an available cluster, return 0
}

// Writes `length` bytes to `file` from `buf`, starting at the file position `skip` bytes
// into the existing file.
// `skip` must be less than the current size of the file.
// The FATFile struct pointed to by `file` will be updated with the new size of the file.
// 
int fat_file_write(FATFile* file, uint8_t* buf, uint32_t length, uint32_t skip) {
    if (length == 0) return -1;
    if (skip > file->file_size) return -1;

    uint32_t bytes_per_cluster = bpb.bytes_per_sector * bpb.sectors_per_cluster;
    
    uint16_t cluster = 0;

    if (skip == 0) {
        cluster = file->start_cluster;
    } else if (skip == file->file_size && skip % bytes_per_cluster == 0) {
        // If our `skip` is at the beginning of a new cluster that hasn't been allocated yet (i.e. is equal to the file size), we need to add that cluster
        // Note: we don't do this for the first cluster, since this is allocated when the file is created.
        uint16_t last_cluster = __get_last_cluster(file);
        if (last_cluster != 0) { 
            cluster = fat_chain_add(last_cluster);
        }
    } else {
        // Otherwise, traverse FAT tree to get existing cluster that is `skip` bytes from start of file
        cluster = __get_cluster_from_offset(file, skip);
    }

    if (cluster == 0) {
        log_error("fat: err: could not find cluster for given position.");
        return -1;
    }

    uint32_t position = skip; // Current position in file, in bytes

    uint8_t* cluster_buf = alloc(bytes_per_cluster);


    bool new_cluster = FALSE;
    while (1) {
        assert_lt_u32(FAT_CLUSTER_EOC_LO, cluster);
        // If we're creating new clusters, rewrite cluster, otherwise, write a new one
        if (!new_cluster) {
            // Todo: optimize, could avoid unnecessary reads if writing whole clusters in middle of file
            position = __cluster_rewrite(cluster, cluster_buf, buf, position, length, skip);
        } else {
            position = __cluster_writenew(cluster, cluster_buf, buf, position, length, skip);
        }

        // When we get to the end of the buffer, one of the above functions will return 0
        if (position == 0) break;
    
        if (fat[cluster] < FAT_CLUSTER_EOC_LO) {
            // If not at end, cluster is next one in chain
            cluster = fat[cluster];
        } else  { // don't add a new cluster if we've reached the end
            // Otherwise find a new unallocated cluster and reserve it
            new_cluster = TRUE;

            cluster = fat_chain_add(cluster);
            if (cluster == 0) {
                // Out of space, error
                log_error("fat12: File allocation table reached max size. Write stopped, file could be corrupted.");
                break;
            }
        }
    };

    free(cluster_buf);

    file->file_size = skip + length;
    // Now we're done writing the file, return success
    return length;
}

FATFile* fat_file_create(char* filename, FATDirectory* directory, uint8_t attributes) {
    // First, allocate space in the FAT
    uint16_t cluster = fat_chain_start();

    // Parse file name for name and extension
    char name[8] = {' '};
    char ext[3] = {' '};
    int ext_idx = -1;
    for (int i = 0; filename[i] != 0 && i < 11; i++) {
        if (filename[i] == '.') {
                ext_idx = i + 1;
        } else if (ext_idx < 0 && i < 8) {
            name[i] = filename[i];
        } else if (ext_idx >= 0 && i - ext_idx < 3) {
            ext[i - ext_idx] = filename[i];
        }
    }
    log_info("Creating file, name and ext:");
    print_count(name, 8);
    println("");
    print_count(ext, 3);
    println("");

    // Next, edit directory table to reflect new file

    FATFile* file = &directory->entries[directory->count];
    file->attributes = attributes;
    memcpy(name, file->filename, 8);
    memcpy(ext, file->extension, 3);
    file->start_cluster = cluster;
    file->file_size = 0;
    
    directory->count++;

    fat_directory_write(directory);

    return file;
}