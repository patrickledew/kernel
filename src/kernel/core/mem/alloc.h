/*
* alloc.h
*
* Memory allocation header file that defines page-based memory management structures
* and functions. It provides a simple paged memory allocation system with bitmap
* tracking of used/free pages and page boundaries. The allocator works with fixed
* page sizes and manages memory within the kernel's address space.
*
* The allocatable region starts at the end of the kernel image and is managed using a bitmap
* to track the status of each page.
*
* Each entry in the bitmap is composed of two bits:
* - The first bit indicates whether the page is used (1) or free (0).
* - The second bit indicates whether the page is the last page in a contiguous block (1) or not (0). This is 
*   needed to know how many pages to free when freeing a block of memory.
*/

#ifndef ALLOC_H
#define ALLOC_H
/*
* Paged memory allocator.
*/

#include "types.h"

#define PAGE_SIZE 0x1000

#define BYTE_IDX(page_idx) (((page_idx) * 2) / 8)
#define BIT_IDX(page_idx) (((page_idx) * 2) % 8)

#define PAGE_USED(page_idx) (bool)(p_bitmap[BYTE_IDX(page_idx)] >> BIT_IDX(page_idx) & 0x01)
#define PAGE_END(page_idx) (bool)(p_bitmap[BYTE_IDX(page_idx)] >> BIT_IDX(page_idx) & 0x10)

#define SET_PAGE_USED(page_idx) p_bitmap[BYTE_IDX(page_idx)] |= (0b01 << BIT_IDX(page_idx));
#define SET_PAGE_END(page_idx) p_bitmap[BYTE_IDX(page_idx)] |= (0b10 << BIT_IDX(page_idx));
                     

// Need to know the address where kernel is loaded, and how big it is

// These are symbols introduced by the linker. They are located at the beginning and end of the kernel image.
extern uint8_t __KERNEL_BEGIN;
extern uint8_t __KERNEL_END;

#define KERNEL_BEGIN &__KERNEL_BEGIN
#define KERNEL_END &__KERNEL_END

void alloc_dbg();
void alloc_init(int pages);
uint8_t* alloc(uint32_t size);
uint32_t free(uint8_t* addr);
uint32_t free_excess(uint8_t* addr);

uint32_t mem_report();

#endif