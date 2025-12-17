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
    return syscall_invoke_1(SYSCALL_OPEN, (int)filename);
}

int close(int fd) {
    return syscall_invoke_1(SYSCALL_CLOSE, fd);
}

int read(int fd, int size, char *dest) {
    return syscall_invoke_3(SYSCALL_READ, fd, size, (int)dest);
}

int write(int fd, int size, char *src) {
    return syscall_invoke_3(SYSCALL_WRITE, fd, size, (int)src);
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