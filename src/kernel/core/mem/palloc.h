#ifndef PALLOC_H
#define PALLOC_H

#include "types.h"
#include "vmem.h"

// Theory of operation:
// This should be able to allocate blocks of physical memory (up to 3GiB ideally)
// and map them to a given virtual memory address in a given page directory.
// 
// ALLOCATING
// Given a size in bytes, and a virtual memory address, we should be able
// to allocate a block of memory of the given size and map the block to the
// given virtual memory address in the given page directory.
//
// FREEING
// Given a virtual memory address, we should be able to free the block of
// memory that starts at that address and is mapped to that address in the
// given page directory. We'd need to do a look up in the page directory for
// the corresponding physical address, and then use that to modify the
// bitmap and free the memory.
//
// STORING
// I was thinking of using an entire page as the bitmap (4KiB),
// but this would only allow us to allocate 0x4000 pages (64 MiB).
// This should be fine for the forseeable future.
// ....that being said....
// If we REALLY DID want the ability to allocate more than 64 MiB,
// A simple solution would be to use a linked list of bitmaps, where
// each bitmap is a page. This would allow us to allocate as many pages
// as needed.
// Each bitmap page would need a header, which would contain a pointer
// to the next bitmap page in the list, and a pointer to the start of the
// bitmap data.
// We may also want to store the bitmap header separately from the bitmap
// pages themselves, e.g. in a fixed length array of bitmap headers.
// This would allow us to easily find the next free bitmap page, and
// would also allow us to easily free a bitmap page.

// ASSUMPTIONS
// - We have allocated 1 page for the physical memory bitmap, which
// indicates which pages in physical memory are available to be allocated.
// 0 indicates a free page, 1 indicates a used page.
// - The page directory passed (and all page tables therein) have not been
// freed, and they are in use, or will be used, by the process whos memory
// is being allocated.
// - When palloc is called, we create a contiguous block of VIRTUAL
// memory. This may not be contiguous in physical memory.
// - palloc will rely on a bit being set on the last page table entry
// in a given block. This bit will be used as a boundary for the block,
// and will be utilized when freeing the block.
// - The caller of palloc will take care to NOT allocate a block over itself.
// I'll probably add a check for this in the future, but right now there's
// no guarantee! This may make it impossible to free all pages from the
// original block.

// OPEN QUESTIONS
// Q: Do we need to use 2 bits for each page?
// Why do we need them for the other alloc? Because that allocates
// contiguous blocks, and we need to know when each block ends.
// In this allocator, we do not allocate contiguous blocks. How
// do we know which pages to free when we give it a starting VMA?
//
// We first look that VMA up in the page table. If we can find it,
// we know where the first physical page is, and hence which bit in
// the bitmap to zero. But, we don't know yet where the next page
// to free would be. It's assumed that if we allocate multiple pages,
// they'd be contiguous in virtual memory. So, if we somehow set a
// bit on the page table entry that terminates that block, we'd know
// when to stop freeing pages.


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
int pfree(uint8_t* vma, uint32_t* page_directory);
int palloc_stat();
uint8_t* palloc_find_next_free();


#endif