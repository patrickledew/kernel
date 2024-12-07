#ifndef MEMORY_H
#define MEMORY_H
/*
* Physical memory manager
*/

#include "types.h"

#define PAGE_SIZE 0x1000

// Need to know the address where kernel is loaded, and how big it is

// These are symbols introduced by the linker. They are located at the beginning and end of the kernel image.
extern uint8_t __KERNEL_BEGIN;
extern uint8_t __KERNEL_END;

extern uint8_t* kernel_begin;
extern uint8_t* kernel_end;

uint8_t* mem_init(int pages);
uint8_t* alloc(uint32_t size);
uint32_t free(uint8_t* addr);

#endif