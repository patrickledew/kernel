#include "syscall.h"
#include "util/logging.h"
#include "core/proc/proc.h"

SYSCALL_DEFINE0(noop) {
    log_info("No-op syscall executed.");
    return 0xFFFFFFFF;
}

SYSCALL_DEFINE1(print, char*, str) {
    print(str);
    return 0;
}

SYSCALL_DEFINE1(exec, char*, path) {
    log_info("Executing process:");
    log_info(path);
    int_enable();
    exec(path);
    return 0;
}

SYSCALL_DEFINE0(yield) {
    // log_number_u("Yielding PID", cur_pid, 10);
    process_yield();
    return 0;
}

syscall_table_entry syscall_table[256] = {
    [0 ... 255] = {syscall_noop, 0},
    __SYSCALL(1, print, 1),
    __SYSCALL(2, exec, 2),
    __SYSCALL(3, yield, 0)

};

void syscall_init() {
    log_info("syscall_init: registering system call ISR.");
    REG_ISR_TRAP(0x80, syscall_handler); // Needs to be traps
}