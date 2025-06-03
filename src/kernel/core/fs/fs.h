/*
 * fs.h
 *
 * This header file defines the core filesystem interface for the kernel. It provides
 * essential file operation functions and structures, particularly focusing on file
 * descriptor management for handling open files. The interface is designed to be
 * filesystem-agnostic while currently implementing FAT12 support.
 */

#ifndef FS_H
#define FS_H

#include "fat/fat12.h"

#define MAX_OPEN_FILES 256
#define MAX_FILE_DESCRIPTORS 256

typedef enum {
    FS_TYPE_FAT12
} FileSystemType;

// Primary struct for storing information about an open file.
typedef struct {
    FATFile* file; // Pointer to filesystem-specific data, e.g., FAT12 file entry
    FATDirectory parent_directory; // Directory containing this file
    FileSystemType fs_type; // Type of filesystem this file is on
    int refcnt; // Reference count for the file, used for managing open files
    bool used;
} File;

// Process-specific file descriptor structure.
typedef struct {
    int ft_idx; // Index into the file table (file_table)
    File* file; // Pointer to the file being accessed
    uint32_t position; // Current position in the file
    uint8_t flags; // Flags for file access (e.g., read, write)
    bool used; // Whether this file descriptor is in use
} FileDescriptor;

// Kernel interface for filesystem operations (manipulating FILES)

// Opens a file and returns an index into the file table.
int file_open(char* filename);
int file_close(int ft_idx);
int file_read(int ft_idx, uint8_t* dest, uint32_t size, uint32_t whence); // Read file at current position
int file_write(int ft_idx, uint8_t* src, uint32_t size, uint32_t whence); // Write file at current position

// Process-specific interface for file operations (manipulating FILE DESCRIPTORS)
// These take in pids to ensure that the file descriptors are managed per process

void fd_table_init(int pid); // Initialize file descriptor table for a process
void fd_table_destroy(int pid); // Destroy file descriptor table for a process

int open(char* filename, uint8_t flags, int pid);
int close(int fd, int pid); // Close file descriptor
int read(int fd, uint8_t* dest, uint32_t size, int pid); // Read from file descriptor at current position
int write(int fd, uint8_t* src, uint32_t size, int pid); // Write to file descriptor at current position
int seek(int fd, uint32_t offset, int pid); // Seek to specific offset from start of file
int tell(int fd, int pid); // Gives the current position in the file

int file_size(int fd); // Returns the size of the file in bytes

#endif