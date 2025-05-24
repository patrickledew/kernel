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

void print(char *str);
void println(char *str);

int exec(char *path);
void yield();
void exit(int status);
int open(char *filename, int flags);
int close(int fd);
int read(int fd, char *buffer, int size);
int write(int fd, char *buffer, int size);
int seek(int fd, int offset);


#endif
    