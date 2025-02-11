#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_VECTOR 0x80

#define SYSCALL_NOOP 0
#define SYSCALL_PRINT 1
#define SYSCALL_EXEC 2
#define SYSCALL_YIELD 3
#define SYSCALL_OPEN 4
#define SYSCALL_CLOSE 5
#define SYSCALL_READ 6
#define SYSCALL_WRITE 7
#define SYSCALL_SEEK 8
#define SYSCALL_EXIT 9

int syscall_invoke_0(int syscall_num);
int syscall_invoke_1(int syscall_num, int arg1);
int syscall_invoke_2(int syscall_num, int arg1, int arg2);


#endif