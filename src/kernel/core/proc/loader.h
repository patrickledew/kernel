#ifndef LOADER_H
#define LOADER_H

#include "types.h"
#include "util/elf.h"
#include "proc.h"


// The program descriptor is used to store information about a loaded
// binary. This is constructed as the binary is loaded into memory and
// page tables are populated to map the binary into the process address.
// The program descriptor is then used to create a process control block.
typedef struct {
    uint8_t* data; // Base address where the program is loaded
    // uint8_t* start; // First address of laoded program
    // uint8_t* end; // Last address of loaded program
    // uint32_t loaded_size; // 
    uint32_t size; // Size of executable file
    uint32_t* page_directory;
    void (*entry_point)(); // In process address space
} ProgramDescriptor;

// Load an ELF file into memory, and return the entry point.
void loader_test(char* path);
int load_program(char* filename, ProgramDescriptor* program);
void load_segment(ProgramDescriptor* program, ElfProgramHeader* ph);
void destroy_program(ProgramDescriptor* program);
#endif