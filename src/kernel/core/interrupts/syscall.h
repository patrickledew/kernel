#ifndef SYSCALL_H
#define SYSCALL_H
#include "types.h"
#include "core/interrupts/interrupts.h"
// idea
// create a syscall table
// have a common syscall entry point
// push registers onto the stack corresponding to different syscall args
// call the syscall according to the table

// The regparm(0) tells gcc to use just the stack when calling these
#define SYSCALL_DEFINE0(name) \
     __attribute__((regparm(0))) int syscall_##name()

#define SYSCALL_DEFINE1(name, type1, name1) \
    __attribute__((regparm(0))) int syscall_##name(type1 name1)
    
#define SYSCALL_DEFINE2(name, type1, name1, type2, name2) \
    __attribute__((regparm(0))) int syscall_##name(type1 name1, type2 name2) \

// Should be put inside of a table intiializer
#define __SYSCALL(number, name, num_args) \
[number] = { syscall_##name, num_args }

typedef struct {
    void* syscall;
    uint8_t num_args;
} syscall_table_entry;

extern syscall_table_entry syscall_table[];

void syscall_init();

__attribute__((interrupt))
extern void syscall_handler(InterruptFrame* frame);

#endif