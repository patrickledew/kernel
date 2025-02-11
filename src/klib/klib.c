// Kernel library functions to be used by programs running on this kernel.
// Abstracts the invocation of syscalls and other functions.

#include "klib.h"
#include "syscall.h"

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
}

int close(int fd) {
    println("close: not implemented.");
    // return syscall_invoke_1(SYSCALL_CLOSE, fd);
}

int read(int fd, char *buffer, int size) {
    println("read: not implemented.");
    // return syscall_invoke_3(SYSCALL_READ, fd, (int)buffer, size);
}

int write(int fd, char *buffer, int size) {
    println("write: not implemented.");
    // return syscall_invoke_3(SYSCALL_WRITE, fd, (int)buffer, size);
}

int seek(int fd, int offset) {
    println("seek: not implemented.");
    // return syscall_invoke_3(SYSCALL_SEEK, fd, offset, offset);
}

void exit(int status) {
    println("exit: not implemented.");
    // syscall_invoke_1(SYSCALL_EXIT, status);
}

