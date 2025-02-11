#include "syscall.h"

int syscall_invoke_0(int syscall_num) {
    int ret;
    __asm__ ("int $0x80; mov %%eax, %0" : "=r"(ret) : "a"(syscall_num));
    return ret;
}

int syscall_invoke_1(int syscall_num, int arg1) {
    int ret; 
    __asm__ ("int $0x80; mov %%eax, %0" : "=r"(ret) : "a"(syscall_num), "b"(arg1));
    return ret;
}

int syscall_invoke_2(int syscall_num, int arg1, int arg2) {
    int ret;
    __asm__ ("int $0x80; mov %%eax, %0" : "=r"(ret) : "a"(syscall_num), "b"(arg1), "c"(arg2));
    return ret;
}