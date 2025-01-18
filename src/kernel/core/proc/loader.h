#ifndef LOADER_H
#define LOADER_H

#include "types.h"
#include "util/elf.h"
#include "proc.h"


// Load an ELF file into memory, and return the entry point.
void loader_test(char* path);
int load_program(char* filename, ProgramDescriptor* program);
void load_segment(ProgramDescriptor* program, ElfProgramHeader* ph);

#endif