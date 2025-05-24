#include "palloc.h"
#include "alloc.h"
#include "memory.h"
#include "util/logging.h"
// PROCESS allocator
// Unlike alloc.c, which is for allocating pages in kernel space, this
// allocator is for allocating pages for user space programs. This allocator
// is used by the process manager to allocate pages for processes.

uint8_t* palloc_bitmap;

// #define PALLOC_DBG

void palloc_init() {
    // Every 2 bits in the bitmap represents a page,
    // so we need 1/4 as many bytes as pages.
    palloc_bitmap = alloc((uint32_t)PAGE_SIZE);
    memfill(palloc_bitmap, PAGE_SIZE, 0);
}

int palloc_map_page(uint8_t* vaddr, uint8_t* paddr, uint32_t* page_directory, bool is_end) {
    uint16_t flags = PAGE_ENTRY_MASK_PRESENT | PAGE_ENTRY_MASK_READWRITE;
    if (is_end) flags |= PAGE_ENTRY_MASK_PALLOC_END;

    #ifdef PALLOC_DBG
    print("palloc_map_page: mapping [");
    print_num((uint32_t)paddr, 16);
    print("->");
    print_num((uint32_t)vaddr, 16);
    println("]");
    #endif

    return vmem_map(page_directory, paddr, vaddr, 1, flags);
}

// Returns:
// -1 if error encountered,
// 1 if the page was successfully unmapped and it was the last page in the block
// and 0 if the page was successfully unmapped and is not the last page
int palloc_unmap_page(uint8_t* vaddr, uint32_t* page_directory) {
    // Get physical address from page table
    uint32_t entry = vmem_entry_get(page_directory, vaddr);
    if (!entry || !(entry & PAGE_ENTRY_MASK_PRESENT)) {
        #ifdef PALLOC_DBG
        log_error("palloc_unmap_page: page already unmapped");
        log_number("palloc_unmap_page: vaddr", vaddr, 16);
        log_number("palloc_unmap_page: entry", entry, 16);
        #endif
        return -1;
    }

    // Get physical address and mark page as free
    uint8_t* paddr = (uint8_t*)(entry & ~0xFFF);
    uint32_t page_idx = P_PAGE_IDX(paddr);
    P_SET_PAGE_FREE(page_idx);


    #ifdef PALLOC_DBG
    print("palloc_unmap_page: unmapping [");
    print_num((uint32_t)paddr, 16);
    print("->");
    print_num((uint32_t)vaddr, 16);
    println("]");
    #endif
    
    int err = vmem_unmap(page_directory, vaddr, 1);
    if (err) {
        #ifdef PALLOC_DBG
        log_error("palloc_unmap_page: failed to unmap page");
        #endif
        return -1;
    } else if (entry & PAGE_ENTRY_MASK_PALLOC_END) {
        // If this is the last page in the block, return 1
        // This is needed to know when to stop freeing consecutive pages
        #ifdef PALLOC_DBG
        log_info("palloc_unmap_page: last page in block unmapped");
        #endif
        return 1;
    }

    return 0;
}

// Returns the physical address, or 0 if out of memory
uint8_t* palloc_find_next_free() {
    for (int page_idx = 0; page_idx < PAGE_SIZE * 8; page_idx++) {
        if (!P_PAGE_USED(page_idx)) {
            return (uint8_t*)(PALLOC_PHYS_BASE + page_idx * PAGE_SIZE);
        }
    }
    return 0;
}
// Returns the next virtual address that's free, according to the page directory.
// Useful for allocating memory in user space.
uint8_t* palloc_find_next_free_v(uint32_t* page_directory) {
 // 1. Iterate over page directory to get page tables
 // NOTE: skips over the first page table, which is reserved (and so we can use 0 as a error value)
    for (int i = 1; i < PAGE_TABLE_ENTRIES; i++) {
        uint32_t entry = page_directory[i];

        if (entry & PAGE_ENTRY_MASK_PRESENT) {
            // This page table is present, iterate over it
            uint32_t* page_table = PADDR_TO_KADDR((uint32_t*)(entry & ~0xFFF));

            for (int j = 0; j < PAGE_TABLE_ENTRIES; j++) {


                if (!(page_table[j] & PAGE_ENTRY_MASK_PRESENT)) {
                    #ifdef PALLOC_DBG
                    log_number("palloc_find_next_free_v: page entry found:", ((i * PAGE_SIZE * PAGE_TABLE_ENTRIES) + (j * PAGE_SIZE)), 16);
                    #endif
                    // This page is free, return the virtual address
                    return (uint8_t*)((i * PAGE_SIZE * PAGE_TABLE_ENTRIES) + (j * PAGE_SIZE));
                }
            }
        } else {
            #ifdef PALLOC_DBG
            log_number("palloc_find_next_free_v: empty page table found:", (i * PAGE_SIZE * PAGE_TABLE_ENTRIES), 16);
            #endif
            // This page table is not present, so all pages in it are free
            return (uint8_t*)(i * PAGE_SIZE * PAGE_TABLE_ENTRIES);
        }
    }
    return 0;  
}

uint8_t* palloc_next_free(uint32_t* page_directory) {
    uint8_t* virt = palloc_find_next_free_v(page_directory);
    if (virt == 0) {
        return 0;
    }
    // Allocate the page
    if (palloc(virt, PAGE_SIZE, page_directory) == -1) {
        return 0;
    }

    return virt;
}



// Returns number of free pages
int palloc_stat() {
    int free_pages = 0;
    for (int page_idx = 0; page_idx < PAGE_SIZE * 8; page_idx++) {
        if (!P_PAGE_USED(page_idx)) {
            free_pages++;
        }
    }
    return free_pages;
}

int palloc(uint8_t* vma, uint32_t size, uint32_t* page_directory) {
    if (vma < (uint8_t*)PALLOC_VIRT_BASE || vma >= (uint8_t*)PALLOC_VIRT_LIMIT) {
        // Outside of palloc virtual range
        return -1;
    }

    // Normalize VMA to page boundary
    vma = (uint8_t*)(((uint32_t)vma) & ~0xFFF);

    // Get number of pages required
    uint32_t pages_required = size / PAGE_SIZE + (size % PAGE_SIZE == 0 ? 0 : 1);
    if (palloc_stat() < pages_required) {
        // Out of memory
        return -1;
    }
    // For each page:
    for (int i = 0; i < pages_required; i++) {
        // Find physical address of next free page
        uint8_t* paddr = palloc_find_next_free();
        if (paddr == 0) {
            // Shouldn't get here, but if we do, panic
            return -1;
        }

        // Map the page
        if (palloc_map_page(vma + i * PAGE_SIZE, paddr, page_directory, i == pages_required - 1) == -1) {
            // Page already mapped, should panic
            return -1;
        }

        // Mark the page as used
        uint32_t page_idx = P_PAGE_IDX(paddr);
        P_SET_PAGE_USED(page_idx);
    }
    // Return success
    return 0;
}

int pfree(uint8_t* vaddr, uint32_t* page_directory) {
    if (vaddr < (uint8_t*)PALLOC_VIRT_BASE || vaddr >= (uint8_t*)PALLOC_VIRT_LIMIT) {
        // Outside of palloc virtual range
        return -1;
    }

    vaddr = (uint8_t*)((uint32_t)vaddr & ~0xFFF);

    while (vaddr < (uint8_t*)PALLOC_VIRT_LIMIT) {
       // Unmap the page
        int ret = palloc_unmap_page(vaddr, page_directory);
        if (ret == -1) {
            // Page not mapped, exit
            return -1;
        } else if (ret == 1) {
            // This is the last page in the block, so we are done
            return 0;
        }


        vaddr += PAGE_SIZE;
    }
    return 0;
}

void palloc_dbg() {
    int free_pages = palloc_stat();
    int total_pages = PAGE_SIZE * 8;
    int used_pages = total_pages - free_pages;
    int col = print_cursor_col_get();
    int row = print_cursor_row_get();
    uint8_t color = print_color_get();
    print_cursor_set(1, 40);
    print_color_set(0x0C);
    print("P: ");
    print_num(used_pages, 10);
    print(" / ");
    print_num(total_pages, 10);
    print_cursor_set(row, col);
    print_color_set(color);
}