#ifndef PROC_H
#define PROC_H

#include "types.h"

typedef struct {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t eip;
    uint32_t eflags;
    uint32_t cs;
    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;
    uint32_t ss;
} Registers;

typedef enum {
    PROC_STATE_RUNNING,
    PROC_STATE_WAITING,
    PROC_STATE_STOPPED
} ProcessState;

typedef struct {
    Registers regs;
    ProcessState state;
    uint32_t pid;
    uint32_t* page_directory;
    uint32_t priority;      // Process priority level
    uint32_t stack_base;     // Stack pointer
    uint32_t stack_size;    // Size of process stack
    uint32_t status;        // Exit status
} __attribute__((packed)) ProcessControlBlock;

#define PROCESS_STACK_SIZE 0x1000

extern void context_switch(int pid);

extern int cur_pid;
extern ProcessControlBlock processes[100];


// Returns the PID of the new process, or -1 if the process could not be created.
int exec(char* path);


void process_exit();
void process_setup_stack(ProcessControlBlock* pcb);
void process_regs_init(ProcessControlBlock* pcb);
void process_switch(ProcessControlBlock* pcb);
void process_destroy(ProcessControlBlock* pcb);
void process_yield();
void process_kill(ProcessControlBlock* pcb);

#endif