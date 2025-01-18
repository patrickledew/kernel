#ifndef PROC_H
#define PROC_H

typedef struct {
    uint8_t* program_memory; // In kernel address space
    uint32_t* page_directory;
    void (*entry_point)(); // In process address space
} ProgramDescriptor;

#endif