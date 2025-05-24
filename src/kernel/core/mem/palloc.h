/*
 * palloc.h
 *
 * Physical memory allocator interface that manages the system's physical memory pages. 
 * It provides functions for allocating, mapping, and freeing physical memory pages, 
 * maintaining a bitmap to track page usage, and handling virtual-to-physical address 
 * translations within page directories. The allocator operates on a fixed memory 
 * region starting at 4MiB past the kernel's base address.
 */

#ifndef PALLOC_H
#define PALLOC_H

#include "types.h"
#include "vmem.h"

// Physical base addr is 4MiB past the start of the kernel (1MiB)
// This is because the kernel's virtual memory mapping is
// 4MB in size, and we don't want to accidentally allocate
// kernel memory

#define PALLOC_LIMIT (0x1000 * 0x1000 * 8) // 64 MiB
#define PALLOC_PHYS_BASE 0x500000
#define PALLOC_PHYS_LIMIT (PALLOC_PHYS_BASE + PALLOC_LIMIT) // Gives us around 3GiB of memory
#define PALLOC_VIRT_BASE 0x00000000
#define PALLOC_VIRT_LIMIT 0xBFFFFFFF // Max virtual address

#define P_BYTE_IDX(page_idx) ((page_idx) / 8)
#define P_BIT_IDX(page_idx) ((page_idx) % 8)
#define P_PAGE_IDX(paddr) (((uint32_t)paddr - PALLOC_PHYS_BASE) / PAGE_SIZE)

#define P_PAGE_USED(page_idx) (bool)(palloc_bitmap[P_BYTE_IDX(page_idx)] >> P_BIT_IDX(page_idx) & 0x01)
#define P_SET_PAGE_USED(page_idx) palloc_bitmap[P_BYTE_IDX(page_idx)] |= (0b1 << P_BIT_IDX(page_idx));
#define P_SET_PAGE_FREE(page_idx) palloc_bitmap[P_BYTE_IDX(page_idx)] &= ~(0b1 << P_BIT_IDX(page_idx));

// Initialize data structures to track allocated regions
void palloc_init();


// Get the physical address corresponding to the given virtual address
// within the given page directory. If the address is not mapped, returns 0.
uint8_t* palloc_get_phys(uint8_t* vma, uint32_t* page_directory);


// Given a size in bytes, and a virtual memory address, this
// allocates a block of memory of the given size and maps the
// block to the given virtual memory address in the given page
// directory. If successful (i.e. enough memory is available),
// returns 0. Otherwise, returns -1.
int palloc(uint8_t* vma, uint32_t size, uint32_t* page_directory);

// Allocates a single page of memory, and returns the virtual address
// of the page. The page is allocated in the given page directory.
// If no pages are available, returns 0.
uint8_t* palloc_next_free(uint32_t* page_directory);

// Frees the page at the given virtual address in the given page directory.
// If the page is not mapped, or another error occurs, returns -1. Otherwise, returns 0.
int pfree(uint8_t* vma, uint32_t* page_directory);

// Returns the number of free pages in the palloc pool
int palloc_stat();

uint8_t* palloc_find_next_free();


void palloc_dbg();



#endif