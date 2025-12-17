#include "fs.h"
#include "util/strutil.h"
#include "util/logging.h"
#include "core/mem/alloc.h"
#include "core/proc/proc.h"
// The file table, used to store information about all
// open files, across all processes.
// indexed using a file table index (ft_idx).
File file_table[MAX_OPEN_FILES];



//
// Kernel interface for filesystem operations (manipulating FILES)
//

// Returns -1 if not found, and otherwise returns the file table index.
int file_open(char* path) {
    // For now, we always assume the FS is FAT12
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
            log_error("file_open: Directory not found:");
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
    FATFile* fat_file;
    for (int i = 0; i < current_dir.count; i++) {
        fat_file = &current_dir.entries[i];
        // Now trim filename and extension of spaces
        char filename_trimmed[9];
        char ext_trimmed[4];
        strcpy(fat_file->filename, filename_trimmed, 8); // Copy filename
        strsplit(filename_trimmed, ' ', filename_trimmed, 0); // Trim spaces
        strcpy(fat_file->extension, ext_trimmed, 3); // Copy extension
        strsplit(ext_trimmed, ' ', ext_trimmed, 0); // Trim spaces

        if (strcmp(filename_trimmed, filename, 8) == 0 // filename must match
         && strcmp(ext_trimmed, ext, 3) == 0 // ext must match
         && (~fat_file->attributes) & FAT_ATTRIBUTE_MASK_SUBDIR) { // not a dir
            found = TRUE;
            break;
        }
    }
    if (!found) {
        log_error("file_open: File not found:");
        log_error(path_segment);
        free((uint8_t*)current_dir.entries);
        return -1;
    }

    // Now add to the file table.

    File file;
    file.parent_directory = current_dir;
    file.file = fat_file; // This was made a pointer so it references the entry in current_dir.entries
    file.used = TRUE;

    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (!file_table[i].used) {
            file_table[i] = file;
            return i;
        }
    }

    log_error("file_open: Max opened files reached.");
    return -2;
} 

int file_close(int ft_idx) {
    if (ft_idx < 0 || ft_idx >= MAX_OPEN_FILES) return -1;
    // Set used to false
    File* file = &file_table[ft_idx];
    if (!file->used) return -1;
    file->used = FALSE;
    // Free entries pointed to by parent directory
    free((uint8_t*)file->parent_directory.entries);
    return 0;
}

int file_read(int ft_idx, uint8_t* dest, uint32_t size, uint32_t whence) {
    if (ft_idx < 0 || ft_idx >= MAX_OPEN_FILES) return -1;
    // Get descriptor from file_table at index fd
    File* file = &file_table[ft_idx];
    if (!file->used) return -1;

    // Actually read the file
    int bytes_read = fat_file_read(file->file, dest, size, whence);
    
    if (bytes_read == -1) return -1; // File read error

    return bytes_read;
}

int file_write(int ft_idx, uint8_t* src, uint32_t size, uint32_t whence) {
    if (ft_idx < 0 || ft_idx >= MAX_OPEN_FILES) return -1;
    // Get descriptor from file_table at index fd
    File* file = &file_table[ft_idx];
    if (!file->used) return -1;

    // Actually write the file
    int bytes_written = fat_file_write(file->file, src, size, whence);
    if (bytes_written == -1) return -1;
    
    // Update directory entry
    int dir_write = fat_directory_write(&file->parent_directory);
    if (dir_write == -1) return -1;

    return bytes_written;
}

//
// Process-specific interface for file operations (manipulating FILE DESCRIPTORS)
//

void fd_table_init(int pid) {

    // Get process
    ProcessControlBlock* pcb = process_get(pid);
    // If the process does not have a file descriptor table, create one
    if (pcb->fd_table) return;

    pcb->fd_table = (FileDescriptor*)malloc(sizeof(FileDescriptor) * MAX_FILE_DESCRIPTORS, pid);
    
    // Initialize the file descriptor table. First 3 are reserved:
    // 0 = stdin
    pcb->fd_table[0].used = TRUE;
    pcb->fd_table[0].file = 0;
    // 1 = stdout
    pcb->fd_table[1].used = TRUE;
    pcb->fd_table[0].file = 0;

    // 2 = stderr
    // Unused

    for (int i = 3; i < MAX_FILE_DESCRIPTORS; i++) {
        pcb->fd_table[i].used = FALSE;
    }
}

void fd_table_destroy(int pid) {
    // Get process file descriptor table
    ProcessControlBlock* pcb = process_get(pid);
    if (!pcb->fd_table) return; // No fd table to destroy

    if (pcb->fd_table[0].file) {
        // Free the ring buffer for stdin
        mfree((uint8_t*)pcb->fd_table[0].file, pid);
        pcb->fd_table[0].file = 0; // Clear pointer
    }

    if (pcb->fd_table[1].file) {
        // Free the ring buffer for stdout
        mfree((uint8_t*)pcb->fd_table[1].file, pid);
        pcb->fd_table[1].file = 0; // Clear pointer
    }

    // Free the file descriptor table
    mfree((uint8_t*)pcb->fd_table, pid);
    pcb->fd_table = 0;
}

int open(char *filename, uint8_t flags, int pid)
{
    // Get process file descriptor table
    ProcessControlBlock* pcb = process_get(pid);
    if (!pcb->fd_table) {
        fd_table_init(pid);
    }

    // TODO check if file already open, otherwise refcnt is useless

    // Open the file using the kernel interface
    int ft_idx = file_open(filename);
    if (ft_idx < 0) {
        log_error("open: Failed to open file:");
        log_error(filename);
        return -1; // File open error
    }

    // Find an unused file descriptor in the process's fd_table
    for (int i = 0; i < MAX_FILE_DESCRIPTORS; i++) {
        if (!pcb->fd_table[i].used) {
            // Initialize the file descriptor
            pcb->fd_table[i].ft_idx = ft_idx; // Store the file table index
            pcb->fd_table[i].file = &file_table[ft_idx];
            pcb->fd_table[i].position = 0;
            pcb->fd_table[i].flags = flags;
            pcb->fd_table[i].used = true;

            // Increment reference count of the file
            file_table[ft_idx].refcnt++;

            return i; // Return the file descriptor index
        }
    }

    return 0;
}
// Close file descriptor
int close(int fd, int pid) {
    if (fd <= 1 || fd >= MAX_FILE_DESCRIPTORS) {
        return -1; // Invalid file descriptor
    }
    // Get process file descriptor table
    ProcessControlBlock* pcb = process_get(pid);
    if (!pcb->fd_table || fd < 0 || fd >= MAX_FILE_DESCRIPTORS) return -1;

    // Get the file descriptor
    FileDescriptor* file_desc = &pcb->fd_table[fd];
    if (!file_desc->used) return -1; // File descriptor not in use

    // Decrement the reference count of the file
    file_desc->file->refcnt--;
    
    // If refcnt is 0, close the file
    if (file_desc->file->refcnt <= 0) {
        file_close(file_desc->ft_idx);
    }

    // Mark the file descriptor as unused
    file_desc->used = false;
    
    return 0; // Success
}

int read_std(FileDescriptor* fd, uint8_t* dest, uint32_t size, int pid) {

    // For stdin/stdout, the file pointer is overridden to point to the ring buffer.
    uint8_t* buffer = (uint8_t*)fd->file;
    if (!buffer) {
        return 0; // Buffer not initialized yet, return 0 bytes read
    }

    for (int i = 0; i < size; i++) {
        // Make sure we don't read past the write position
        if (fd->position == fd->write_position) {
            return i; // Buffer is empty, return number of bytes read
        }

        // Read from the ring buffer
        dest[i] = buffer[fd->position];
        fd->position = (fd->position + 1) % PAGE_SIZE; // Wrap around
    }

    return size; // Return number of bytes read
}

int write_std(FileDescriptor* fd, uint8_t* src, uint32_t size, int pid) {
    uint8_t* buffer = (uint8_t*)fd->file;
    if (!fd->file) {
        // Buffer not initialized yet, allocate and initialize it
        buffer = fd->file = malloc(PAGE_SIZE, pid); // Allocate a page for the ring buffer
        // log_info("write_std: Allocated ring buffer for stdin/stdout.");
        // log_number("Ring buffer address", (uint32_t)buffer, 16);
    }
    for (int i = 0; i < size; i++) {
        // // Make sure we don't write past the read position (from the back)
        // if ((fd->write_position + 1) % PAGE_SIZE == fd->position) {
        //     return i; // Buffer is full, return number of bytes written
        // }
        // Write to the ring buffer
        buffer[fd->write_position] = src[i];
        
        fd->write_position = (fd->write_position + 1) % PAGE_SIZE; // Wrap around
    }
    return size; // Return number of bytes written
}
// Read from file descriptor at current position
int read(int fd, uint8_t* dest, uint32_t size, int pid) {
    if (fd < 0 || fd >= MAX_FILE_DESCRIPTORS) {
        return -1; // Invalid file descriptor
    }

    // Get process file descriptor table
    ProcessControlBlock* pcb = process_get(pid);
    if (!pcb->fd_table) fd_table_init(pid);
    
    // Get the file descriptor
    FileDescriptor* file_desc = &pcb->fd_table[fd];
    if (!file_desc->used) return -1; // File descriptor not in use
    
    int bytes_read = 0;
    if (fd <= 1) {
        bytes_read = read_std(file_desc, dest, size, pid);
    } else {
        // Read from the file
        bytes_read = file_read(file_desc->ft_idx, dest, size, file_desc->position);
    }

    if (bytes_read < 0) return -1; // Read error

    if (fd > 1) {
        // Update the position in the file descriptor - note, this is done in write_std for stdin/stdout
        file_desc->position += bytes_read;
    }

    return bytes_read; // Return number of bytes read
}

// Write to file descriptor at current position
int write(int fd, uint8_t* src, uint32_t size, int pid) {
    if (fd < 0 || fd >= MAX_FILE_DESCRIPTORS) {
        return -1; // Invalid file descriptor
    }

    // Get process file descriptor table
    ProcessControlBlock* pcb = process_get(pid);
    if (!pcb->fd_table) fd_table_init(pid);
    
    // Get the file descriptor
    FileDescriptor* file_desc = &pcb->fd_table[fd];
    if (!file_desc->used) return -1; // File descriptor not in use

    // Write to the file
    int bytes_written;
    if (fd <= 1) {
        bytes_written = write_std(file_desc, src, size, pid);
    } else {
        // Read from the file
        bytes_written = file_write(file_desc->ft_idx, src, size, file_desc->write_position);
    }
    if (bytes_written < 0) return -1; // Write error

    if (fd > 1) {
        // Update the position in the file descriptor
        file_desc->write_position += bytes_written;
    }
    return bytes_written; // Return number of bytes written
}


int seek(int fd, uint32_t offset, int pid) {
    ProcessControlBlock* pcb = process_get(pid);

    if (!pcb->fd_table || fd < 0 || fd >= MAX_OPEN_FILES) return -1;
    // Get descriptor from file_table at index fd
    FileDescriptor* file_desc = &pcb->fd_table[fd];
    if (!file_desc->used) return -1;

    if (offset >= file_desc->file->file->file_size) return -2;

    file_desc->position = offset;

    return 0;
}

int tell(int fd, int pid) {
    ProcessControlBlock* pcb = process_get(pid);
    if (!pcb->fd_table || fd < 0 || fd >= MAX_FILE_DESCRIPTORS) return -1;

    FileDescriptor* file_desc = &pcb->fd_table[fd];
    if (!file_desc->used) return -1;
    
    return file_desc->position;
}

int file_size(int ft_idx) {
    if (ft_idx < 0 || ft_idx >= MAX_OPEN_FILES) return -1;

    File* file = &file_table[ft_idx];
    if (!file->used) return -1;

    return file->file->file_size;
}