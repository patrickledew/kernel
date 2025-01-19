#include "core/mem/alloc.h"
#include "util/logging.h"
#include "core/vga/vga.h"
/*
* Physical Memory Manager
*
* This will allow allocating blocks of memory starting from the end of the loaded kernel image, plus what we allocate for the bitmap.
*/

uint32_t num_pages; // number of pages available to be allocated
uint32_t pages_free;
// bitmap used to allocate pages.
// format: this bitmap contains a two-bit pair for each page.
// the first (least significant) bit is the used(1)/free(0) bit, and the second is the end marker bit.
//  If the end marker bit is set, it is the last page in a contiguous block.
// e.g. 00000000 1010101010101011 0000 1010111 0101011
//      ^free    ^8 used pages    ^free
uint8_t* p_bitmap;
uint8_t* p_start; // start of allocatable region

// alloc_init
// Initializes a bitmap to keep track of which pages are free or used.
// This locates the bitmap at the end of the kernel address space.
void alloc_init(int pages)
{
    log_info("mem: initializing paged memory manager");
    num_pages = pages;
    pages_free = pages;

    // Construct the bitmap starting at the end of the loaded kernel
    uint8_t* ptr = (uint8_t*)KERNEL_END;
    p_bitmap = ptr;
    while (ptr < KERNEL_END + pages / 8 + (pages % 8 == 0 ? 0 : 1)) {
        *ptr = 0x00;
        ptr++;
    }
    // p_start will be on the first page boundary after the bitmap
    uint32_t ptr_u32 = (uint32_t)ptr;
    p_start = (uint8_t*)(ptr_u32 % 0x1000 == 0 
                        ? ptr_u32 
                        : ptr_u32 + (0x1000 - ptr_u32 % 0x1000));

}

uint32_t mem_report() {
    return pages_free;
}

// TODO refactor to use new macros
// Allocates [size] bytes of physical memory and returns a pointer to the start of the reserved block.
uint8_t* alloc(uint32_t size) {
    uint32_t pages_required = size / PAGE_SIZE + (size % PAGE_SIZE == 0 ? 0 : 1);
    uint32_t start_page = 0;
    uint32_t pages_found = 0;
    bool success = FALSE;
    // First, find a section of contiguous pages large enough to hold [size]
    for (uint32_t i = 0; i < num_pages; i++) {
        uint32_t byte_idx = (i * 2) / 8;
        uint32_t bit_idx = (i * 2) % 8;
        uint8_t byte = p_bitmap[byte_idx];
        
        bool used = (bool)((byte >> bit_idx) & 0b01); // Check if used bit set for page i
        bool end = (bool)((byte >> bit_idx) & 0b10); // Check if free bit set for page i

        if (used) { // If page already used, restart search - set start page to next page
            pages_found = 0;
            start_page = i + 1;
        } else {
            pages_found++; // If unused, increment number of contiguous pages found
        }

        if (pages_found == pages_required) { // Once we reach the number of pages we need, claim those pages and return pointer
            success = TRUE;
            break;
        }
    }
    
    if (!success) return 0;

    // If successful, reserve all bits in contiguous block
    for (uint32_t i = start_page; i < start_page + pages_found; i++) {
        uint32_t byte_idx = (i * 2) / 8;
        uint32_t bit_idx = (i * 2) % 8;
        bool isEnd = i == start_page + pages_found - 1;
        if (isEnd) {
            p_bitmap[byte_idx] |= (0b11 << bit_idx); // Set end and used bits for page i
        } else {
            p_bitmap[byte_idx] |= (0b01 << bit_idx); // Set used bit for page i
        }

        pages_free -= 1;
    }

    log_number_u("mem: allocated from ", p_start + start_page * PAGE_SIZE, 16);
    log_number_u("mem: to", p_start + (start_page + pages_required) * PAGE_SIZE, 16);

    return p_start + start_page * PAGE_SIZE;   
}

// Returns number of pages freed
uint32_t free(uint8_t* addr) {
    // Check bounds
    if (addr < p_start || addr >= p_start + (num_pages * PAGE_SIZE)) return 0;


    // Calculate start page index
    uint32_t start_page = (addr - p_start) / PAGE_SIZE;

    log_number_u("mem: dealloc starting from ", p_start + start_page * PAGE_SIZE, 16);

    for (uint32_t i = start_page; i < num_pages; i++) {
        uint32_t byte_idx = (i * 2) / 8;
        uint32_t bit_idx = (i * 2) % 8;
        uint8_t byte = p_bitmap[byte_idx];
        
        bool used = (bool)((byte >> bit_idx) & 0b01); // Check if used bit set for page i
        bool end = (bool)((byte >> bit_idx) & 0b10); // Check if free bit set for page i
 
        // ALl bits we encounter should be used. 
        // If we encounter this, either caller passed a bad pointer
        // or the bitmap is missing an end bit for one of the blocks.
        if (!used)
            return 0;

        p_bitmap[byte_idx] &= ~(0b11 << bit_idx); // Clear bits for page i
        
        pages_free += 1;

        if (end)
            return i - start_page * PAGE_SIZE;
    }
}

// Should free excess unused pages after addr. If addr is in the middle of a page, should not free that page.
uint32_t free_excess(uint8_t* addr) {
    if (addr < p_start || addr >= p_start + (num_pages * PAGE_SIZE)) return 0;

    // Get index of page which contains the address
    int page_idx = (addr - p_start) / PAGE_SIZE;
    int free_same = (addr - p_start) % PAGE_SIZE == 0; // If on page boundary, also free current page

    // If current page unused, doesnt belong to allocated block
    if (!PAGE_USED(page_idx)) return 0; 

    if (free_same) {
        // We need to mark the previous page as end (if used and not already end)
        if (page_idx != 0 && PAGE_USED(page_idx - 1)){ // If prev page used, mark as end
            SET_PAGE_END(page_idx - 1);
        }
        // then we need to free pages, starting with the one that includes the addr given
        return free(addr);
    } else {
        // We need to mark the current page as end, then free pages starting from the next page
        // Already know current page is used, so we can mark as end
        
        if (PAGE_END(page_idx)) return 0; // If current page already marked as end, next page is not part of current block

        SET_PAGE_END(page_idx);
        return free((page_idx + 1) * PAGE_SIZE + p_start); // Free remaining pages starting from next page
    }
}

