/*
 * syscall.h
 *
 * Header file defining system call numbers and prototypes for invoking system
 * calls in the kernel. Contains constants for different system call types and
 * declarations for functions that handle system calls with different numbers
 * of arguments.
 */

#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_VECTOR 0x80

#define SYSCALL_NOOP 0
#define SYSCALL_PRINT 1
#define SYSCALL_EXEC 2
#define SYSCALL_YIELD 3
#define SYSCALL_EXIT 4
#define SYSCALL_MALLOC 5
#define SYSCALL_FREE 6
#define SYSCALL_OPEN 7
#define SYSCALL_CLOSE 8
#define SYSCALL_READ 9
#define SYSCALL_WRITE 10
#define SYSCALL_SEEK 11

int syscall_invoke_0(int syscall_num);
int syscall_invoke_1(int syscall_num, int arg1);
int syscall_invoke_2(int syscall_num, int arg1, int arg2);
int syscall_invoke_3(int syscall_num, int arg1, int arg2, int arg3);


#endif