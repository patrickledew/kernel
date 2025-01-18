#include "fs.h"
#include "util/strutil.h"
#include "util/logging.h"
#include "core/mem/alloc.h"

FileDescriptor fd_table[MAX_OPEN_FILES];


// Returns -1 if not found, and otherwise returns the file descriptor.
int open(char* path, uint8_t flags) {
    FATDirectory current_dir = fat_directory_read(0);
    // 1. Parse filename, need strcmp and possibly strsplit function for this
    char path_segment[32];
    int offset = 0;
    while (1) {
        offset = strsplit(path, '/', path_segment, offset);
        if (strlen(path_segment) == 0) continue; // Ignore whatever's before the first /, or whats between double slashes //
        if (offset < 0) break;
        // path_segment is name of directory, try and find that in current dir
        bool found = FALSE;
        for (int i = 0; i < current_dir.count; i++) {
            FATFile entry = current_dir.entries[i];
            char filename_trimmed[9] = {0};

            strcpy(entry.filename, filename_trimmed, 8); // Copy filename
            strsplit(filename_trimmed, ' ', filename_trimmed, 0); // Trim spaces

            // log_info(filename_trimmed);

            if (strcmp(filename_trimmed, path_segment, 0) == 0 && entry.attributes & FAT_ATTRIBUTE_MASK_SUBDIR) {
                // Matched filename. Try reading directory.
                found = TRUE;
                // First free entries of parent directory, as we won't need it anymore and dont want a memory leak
                free((uint8_t*)current_dir.entries);
                current_dir = fat_directory_read(entry.start_cluster);
                break;
            }
        }
        if (!found){
            log_error("open: Directory not found:");
            log_error(path_segment);
            free((uint8_t*)current_dir.entries);
            return -1;
        }
    }

    // Now path_segment should be name of file, parse for filename and extension
    char filename[9] = {0};
    char ext[4] = {0};
    offset = strsplit(path_segment, '.', filename, 0);

    if (offset > 0) { // If we have an extension, copy to ext
        strcpy(path_segment + offset, ext, 3);
    }

    // Now compare to see if we find the file
    bool found = FALSE;
    FATFile* file;
    for (int i = 0; i < current_dir.count; i++) {
        file = &current_dir.entries[i];
        // Now trim filename and extension of spaces
        char filename_trimmed[9];
        char ext_trimmed[4];
        strcpy(file->filename, filename_trimmed, 8); // Copy filename
        strsplit(filename_trimmed, ' ', filename_trimmed, 0); // Trim spaces
        strcpy(file->extension, ext_trimmed, 3); // Copy extension
        strsplit(ext_trimmed, ' ', ext_trimmed, 0); // Trim spaces

        if (strcmp(filename_trimmed, filename, 8) == 0 // filename must match
         && strcmp(ext_trimmed, ext, 3) == 0 // ext must match
         && (~file->attributes) & FAT_ATTRIBUTE_MASK_SUBDIR) { // not a dir
            found = TRUE;
            break;
        }
    }
    if (!found) {
        log_error("open: File not found:");
        log_error(path_segment);
        free((uint8_t*)current_dir.entries);
        return -1;
    }

    // Now we have the file. Create the file descriptor!

    FileDescriptor fd;
    fd.parent_directory = current_dir;
    fd.file = file; // This was made a pointer so it references the entry in current_dir.entries
    fd.position = 0;
    fd.used = TRUE;

    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (!fd_table[i].used) {
            fd_table[i] = fd;
            return i;
        }
    }

    log_error("open: Max opened files reached.");
    return -2;
} 

int close(int fd) {
    if (fd < 0 ||fd >= MAX_OPEN_FILES) return -1;
    // Set used to false
    FileDescriptor* file = &fd_table[fd];
    if (!file->used) return -1;
    file->used = FALSE;
    free((uint8_t*)file->parent_directory.entries);
    // Free entries pointed to by parent directory
    return 0;
}

int read(int fd, uint8_t* dest, uint32_t size) {
    if (fd < 0 || fd >= MAX_OPEN_FILES) return -1;
    // Get descriptor from fd_table at index fd
    FileDescriptor* file = &fd_table[fd];
    if (!file->used) return -1;

    // Actually read the file
    int bytes_read = fat_file_read(file->file, dest, size, file->position);
    
    if (bytes_read == -1) return -1; // File read error

    // Update position
    file->position += bytes_read;
    if (file->position > file->file->file_size) {
        file->position = file->file->file_size;
    }
    return 0;
}

int write(int fd, uint8_t* src, uint32_t size) {
    if (fd < 0 || fd >= MAX_OPEN_FILES) return -1;
    // Get descriptor from fd_table at index fd
    FileDescriptor* file = &fd_table[fd];
    if (!file->used) return -1;

    // Actually write the file
    int bytes_written = fat_file_write(file->file, src, size, file->position);
    if (bytes_written == -1) return -1;
    
    // Update directory entry
    int dir_write = fat_directory_write(&file->parent_directory);
    if (dir_write == -1) return -1;

    // Update position
    file->position += bytes_written;
    if (file->position > file->file->file_size) {
        file->position = file->file->file_size;
    }
    return 0;
}
int seek(int fd, uint32_t offset) {
    if (fd < 0 || fd >= MAX_OPEN_FILES) return -1;
    // Get descriptor from fd_table at index fd
    FileDescriptor* file = &fd_table[fd];
    
    if (!file->used) return -1;
    if (offset >= file->file->file_size) return -2;

    file->position = offset;

    return 0;
}

int tell(int fd) {
    if (fd < 0 || fd >= MAX_OPEN_FILES) return -1;

    FileDescriptor* file = &fd_table[fd];
    if (!file->used) return -1;
    
    return file->position;
}

int fsize(int fd) {
    if (fd < 0 || fd >= MAX_OPEN_FILES) return -1;

    FileDescriptor* file = &fd_table[fd];
    if (!file->used) return -1;

    return file->file->file_size;
}