/*
 * klib.h
 *
 * This header file defines the interface for the kernel library, which provides
 * user-space programs with a standard set of system calls and utility functions
 * for interacting with the kernel. It includes basic I/O operations, process
 * management, and file system access functions that abstract the underlying
 * system call mechanisms.
 */

#ifndef KLIB_H
#define KLIB_H
// Kernel library functions to be used by programs running on this kernel.
// Abstracts the invocation of syscalls and other functions.
#include <stddef.h>

void print(char *str);
void println(char *str);

int exec(char *path);
void yield();
void exit();

void* malloc(size_t size);
int free(char* ptr);

int open(char *filename, int flags);
int close(int fd);
int read(int fd, int size, char* dest);
int write(int fd, int size, char* src);
int seek(int fd, int offset);

#endif
    