// Kernel library functions to be used by programs running on this kernel.
// Abstracts the invocation of syscalls and other functions.

#include "klib.h"
#include "syscall.h"
#include <stddef.h>

void print(char *str) {
    syscall_invoke_1(SYSCALL_PRINT, (int)str);
}
void println(char *str) {
    print(str);
    print("\n");
}

int exec(char *path) {
    return syscall_invoke_1(SYSCALL_EXEC, (int)path);
}

void yield() {
    syscall_invoke_0(SYSCALL_YIELD);
}

int open(char *filename, int flags) {
    println("open: not implemented.");
    // return syscall_invoke_2(SYSCALL_OPEN, (int)filename, flags);
    return -1;
}

int close(int fd) {
    println("close: not implemented.");
    // return syscall_invoke_1(SYSCALL_CLOSE, fd);
    return -1;
}

int read(int fd, char *buffer, int size) {
    println("read: not implemented.");
    // return syscall_invoke_3(SYSCALL_READ, fd, (int)buffer, size);
    return -1;
}

int write(int fd, char *buffer, int size) {
    println("write: not implemented.");
    // return syscall_invoke_3(SYSCALL_WRITE, fd, (int)buffer, size);
    return -1;
}

int seek(int fd, int offset) {
    println("seek: not implemented.");
    // return syscall_invoke_3(SYSCALL_SEEK, fd, offset, offset);
    return -1;
}

void exit() {
    syscall_invoke_0(SYSCALL_EXIT);
}

void* malloc(size_t size) {
    return (void*)syscall_invoke_1(SYSCALL_MALLOC, size);
}

int free(char* ptr) {
    return (void*)syscall_invoke_1(SYSCALL_FREE, ptr);
}