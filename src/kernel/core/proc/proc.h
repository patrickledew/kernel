#ifndef PROC_H
#define PROC_H

#include "types.h"

// The program descriptor is used to store information about a loaded
// binary. This is constructed as the binary is loaded into memory and
// page tables are populated to map the binary into the process address.
// The program descriptor is then used to create a process control block.
typedef struct {
    uint8_t* base; // Base address where the program is loaded
    uint32_t size; // Size of executable file
    uint32_t* page_directory;
    void (*entry_point)(); // In process address space
} ProgramDescriptor;

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
    uint32_t pid;
    ProcessState state;
    uint8_t* base; // Base address of process memory
    uint32_t limit; // Size of process memory
    uint32_t* page_directory;
    Registers regs;
    uint32_t parent_pid;    // Parent process ID
    uint32_t priority;      // Process priority level
    uint32_t stack_base;     // Stack pointer
    uint32_t stack_limit;    // Size of process stack
    uint32_t status;        // Exit status
} ProcessControlBlock;

// Returns the PID of the new process, or -1 if the process could not be created.
int exec(char* path);

#endif