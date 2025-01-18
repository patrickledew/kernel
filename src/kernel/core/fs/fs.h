#ifndef FS_H
#define FS_H
// Main file for filesystem operations. Utilizes file descriptors to keep track of opened files.

#include "fat/fat12.h"

#define MAX_OPEN_FILES 256

// Primary struct for storing information about an open file.
typedef struct {
    bool used;
    uint32_t position;
    FATDirectory parent_directory;
    FATFile* file;
} FileDescriptor;

int open(char* filename, uint8_t flags);
int close(int fd);

int read(int fd, uint8_t* dest, uint32_t size); // Read file at current position
int write(int fd, uint8_t* src, uint32_t size); // Write file at current position
int seek(int fd, uint32_t offset); // Seek to specific offset from start of file
int tell(int fd); // Gives the current position in the file

int fsize(int fd); // Returns the size of the file in bytes

#endif