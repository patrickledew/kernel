#include "syscall.h"
#include "util/logging.h"

SYSCALL_DEFINE0(noop) {
    log_info("No-op syscall executed.");
    return 0xFFFFFFFF;
}

SYSCALL_DEFINE1(print, char*, str) {
    print(str);
    return 0;
}

SYSCALL_DEFINE2(two, char*, first, char*, second) {
    log_info("First string:");
    log_error(first);
    log_info("Second string:");
    log_error(second);
    return 0;
}

syscall_table_entry syscall_table[256] = {
    [0 ... 255] = {syscall_noop, 0},
    __SYSCALL(1, print, 1),
    __SYSCALL(2, two, 2),

};

void syscall_init() {
    log_info("syscall_init: registering system call ISR.");
    REG_ISR(0x80, syscall_handler);
}