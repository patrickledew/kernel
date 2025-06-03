/*
 * proc.h
 *
 * Defines process management structures and functions for the kernel. This header 
 * file contains the ProcessControlBlock structure which manages process state,
 * interface for process creation, switching, and termination operations essential
 * including registers, memory allocation, and execution status. It provides the 
 * for the kernel's multitasking capabilities.
 */

#ifndef PROC_H
#define PROC_H

#include "types.h"
#include "core/fs/fs.h"

typedef struct ProcessControlBlock ProcessControlBlock;

#include "core/mem/malloc.h"

typedef struct {
    uint32_t eax; // +0
    uint32_t ebx; // +4
    uint32_t ecx; // +8
    uint32_t edx; // +12
    uint32_t esi; // +16
    uint32_t edi; // +20
    uint32_t ebp; // +24
    uint32_t esp; // +28 <-.
    uint32_t eip; // +32  -'
    uint32_t eflags; // +36
    uint32_t cs; // +40
    uint32_t ds; // +44
    uint32_t es; // +48
    uint32_t fs; // +52
    uint32_t gs; // +56
    uint32_t ss; // +60
} Registers;

typedef enum {
    PROC_STATE_STOPPED,
    PROC_STATE_RUNNING,
    PROC_STATE_WAITING
} ProcessState;

typedef struct ProcessControlBlock {
    Registers regs; // +0
    ProcessState state; // +64
    uint32_t pid; // +68
    uint32_t* page_directory; // +72
    uint32_t parent_pid;      // Process priority level
    uint32_t stack_base;     // Stack pointer
    uint32_t stack_size;    // Size of process stack
    uint32_t status;        // Exit status
    Heap* heap; // Pointer to info needed by malloc - will be initialized on first call to malloc()
    FileDescriptor* fd_table; // Pointer to the process's file descriptor table
} __attribute__((packed)) ProcessControlBlock;

#define PROCESS_STACK_SIZE 0x1000
#define MAX_PROCESSES 100

extern void context_switch(int pid);

extern int cur_pid;
extern ProcessControlBlock processes[MAX_PROCESSES];


// Returns the PID of the new process, or -1 if the process could not be created.
int process_exec(char* path);

ProcessControlBlock* process_get(int pid);
void process_exit();
void process_setup_stack(ProcessControlBlock* pcb);
void process_regs_init(ProcessControlBlock* pcb);
void process_switch(ProcessControlBlock* pcb);
void process_destroy(ProcessControlBlock* pcb);
void process_yield();
void process_kill(ProcessControlBlock* pcb);

#endif