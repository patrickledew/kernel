#include "syscall.h"
#include "util/logging.h"
#include "core/proc/proc.h"
#include "util/panic.h"

SYSCALL_DEFINE0(noop) {
    panic("No-op syscall executed.");
    return 0xFFFFFFFF;
}

SYSCALL_DEFINE1(print, char*, str) {
    print(str);
    return 0;
}

SYSCALL_DEFINE1(exec, char*, path) {
    log_info("Executing process:");
    log_info(path);
    process_exec(path);
    return 0;
}

SYSCALL_DEFINE0(yield) {
    // log_number_u("Yielding PID", cur_pid, 10);
    process_yield();
    return 0;
}

// TODO maybe make this take a status code
SYSCALL_DEFINE0(exit) {
    log_number("Exiting PID", cur_pid, 10);
    process_exit();
    return 0;
}

SYSCALL_DEFINE1(malloc, size_t, size) {
    log_number("malloc syscall executed by pid ", cur_pid, 10);
    uint8_t* addr = malloc(size, cur_pid);
    return addr;
}

SYSCALL_DEFINE1(free, uint8_t*, addr) {
    log_number("free syscall executed by pid ", cur_pid, 10);
    int status = mfree(addr, cur_pid);
    return status;
}

SYSCALL_DEFINE1(open, char*, filename) {
    log_info("syscall_open:");
    log_info(filename);
    int fd = open(filename, 0, cur_pid);
    return fd;
}

SYSCALL_DEFINE1(close, int, fd) {
    log_number("syscall_close: fd", fd, 10);
    int status = close(fd, cur_pid);
    return status;
}

SYSCALL_DEFINE3(read, int, fd, uint32_t, size, uint8_t*, dest) {
    int bytes_read = read(fd, dest, size, cur_pid);
    // For simplicity, we just return the number of bytes read
    return bytes_read;
}

SYSCALL_DEFINE3(write, int, fd, uint32_t, size, uint8_t*, src) {
    log_number("syscall_write: fd", fd, 10);
    int bytes_written = write(fd, src, size, cur_pid);
    // For simplicity, we just return the number of bytes written
    return bytes_written;
}

syscall_table_entry syscall_table[256] = {
    [0 ... 255] = {syscall_noop, 0},
    __SYSCALL(1, print, 1),
    __SYSCALL(2, exec, 2),
    __SYSCALL(3, yield, 0),
    __SYSCALL(4, exit, 0),
    __SYSCALL(5, malloc, 1),
    __SYSCALL(6, free, 1),
    __SYSCALL(7, open, 1),
    __SYSCALL(8, close, 1),
    __SYSCALL(9, read, 3),
    __SYSCALL(10, write, 3),
};

void syscall_init() {
    log_info("syscall_init: registering system call ISR.");
    REG_ISR_TRAP(0x80, syscall_handler); // Needs to be traps
}