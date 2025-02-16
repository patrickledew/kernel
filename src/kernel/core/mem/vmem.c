#include "vmem.h"
#include "core/mem/alloc.h"
#include "core/mem/memory.h"
#include "util/logging.h"
#include "util/assert.h"

uint32_t* kernel_page_directory;
uint32_t* kernel_page_table;
uint32_t* current_page_directory;

void vmem_init() {
    kernel_page_directory = (uint32_t*)((uint32_t)_KERNEL_PAGE_DIRECTORY + 0xC0000000 - 0x100000);
    kernel_page_table = (uint32_t*)((uint32_t)_KERNEL_PAGE_TABLE + 0xC0000000 - 0x100000);
    current_page_directory = kernel_page_directory;
    log_info("vmem_init: cleaning up memory map.");
    vmem_load(kernel_page_directory);
    vmem_zap_identity();
}

void vmem_zap_identity() {
     // Overwrite first page directory entry
     // this is the identity entry that maps 0x0 - 0x400000 to itself
    kernel_page_directory[0] = 0;
    vmem_load(kernel_page_directory);
}

void vmem_load(uint32_t* page_directory) {
    assert_gt_u32(0xc0000000, (uint32_t)page_directory);
    vmem_load_absolute((uint32_t*)KADDR_TO_PADDR(page_directory));
    current_page_directory = page_directory;
}

void vmem_load_absolute(uint32_t* page_directory) {
    // Make sure we are loading from a high memory address
    // No PTs allowed in low memory
    // log_number_u("Loading page directory at PA", (uint32_t)page_directory, 16);
    __asm__("mov %0, %%cr3" :: "a"(page_directory));
}


// Creates a default page directory with the kernel already mapped.
uint32_t* vmem_pd_create() {
        uint32_t* page_directory = (uint32_t*)alloc(PAGE_TABLE_ENTRIES * 4);
        memfill((uint8_t*)page_directory, 4096, 0);
        // Use existing kernel page table
        uint8_t* kernel_pt_paddr = KADDR_TO_PADDR(kernel_page_table);
        page_directory[0x300] = (uint32_t)kernel_pt_paddr | PAGE_ENTRY_MASK_PRESENT | PAGE_ENTRY_MASK_READWRITE ;

        return page_directory;
}
 // Frees all allocated page tables in a page dir, and frees the page dir itself
void vmem_pd_destroy(uint32_t* page_directory) {
    for (int i = 0; i < PAGE_TABLE_ENTRIES; i++) {
        if (page_directory[i] & PAGE_ENTRY_MASK_PRESENT) {
            uint32_t* page_table = (uint32_t*)PADDR_TO_KADDR(page_directory[i] & 0xFFFFF000);
            if (page_table == kernel_page_table) continue; // We remap this in every page directory, we don't ever want to free it
            free((uint8_t*)page_table);
        }
    }
    free((uint8_t*)page_directory);
}


// Returns 0 on success, -1 if page already mapped.
int vmem_map(uint32_t* page_directory, uint8_t* p_addr, uint8_t* v_addr, uint32_t num_pages, uint16_t flags) {
    assert_u32(0, (uint32_t)page_directory % 0x1000);
     // align to page boundaries
    uint32_t v_addr_aligned = (uint32_t)v_addr & 0xFFFFF000;
    uint32_t p_addr_aligned = (uint32_t)p_addr & 0xFFFFF000;
    for (int i = 0; i < num_pages; i++) {
        uint32_t pd_idx = (v_addr_aligned / PAGE_SIZE + i) / PAGE_TABLE_ENTRIES;
        uint32_t pt_idx = (v_addr_aligned / PAGE_SIZE + i) % PAGE_TABLE_ENTRIES;
        uint32_t* page_table;
        if (page_directory[pd_idx] & PAGE_ENTRY_MASK_PRESENT) {
            page_table = (uint32_t*)PADDR_TO_KADDR(page_directory[pd_idx] & 0xFFFFF000);
            uint32_t page_table_entry = page_table[pt_idx];
            
            if (page_table_entry & PAGE_ENTRY_MASK_PRESENT) return -1; // Page already mapped
        } else {
            // allocate new page table
            page_table = (uint32_t*)alloc(0x1000);
            // Must be on page boundary
            assert_u32(0, (uint32_t)page_table % 0x1000);

            memfill((uint8_t*)page_table, 0x1000, 0); // We need to zero out existing memory
            
            // Update page table directory entry
            page_directory[pd_idx] = (uint32_t)KADDR_TO_PADDR(page_table) | PAGE_ENTRY_MASK_PRESENT | PAGE_ENTRY_MASK_READWRITE;
        }
        // Populate page table entry
        page_table[pt_idx] = p_addr_aligned | PAGE_ENTRY_MASK_PRESENT | PAGE_ENTRY_MASK_READWRITE | flags;
    }

    return 0;
}


// Returns 0 if all pages were successfully unmapped,
// and -1 if any unmapped pages were encountered.
// Stops unmapping when the first unmapped page is encountered.
int vmem_unmap(uint32_t* page_directory, uint8_t* v_addr, uint32_t num_pages) {
    uint32_t v_addr_aligned = (uint32_t)v_addr & 0xFFFFF000;
    for (int i = 0; i < num_pages; i++) {
        uint32_t pd_idx = (v_addr_aligned / PAGE_SIZE + i) / PAGE_TABLE_ENTRIES;
        uint32_t pt_idx = (v_addr_aligned / PAGE_SIZE + i) % PAGE_TABLE_ENTRIES;
        if (page_directory[pd_idx] & PAGE_ENTRY_MASK_PRESENT) {
            uint32_t* page_table = (uint32_t*)PADDR_TO_KADDR(page_directory[pd_idx] & 0xFFFFF000);
            if (page_table[pt_idx] & PAGE_ENTRY_MASK_PRESENT) {
                page_table[pt_idx] = 0;
            } else {
                return -1; // Page not mapped
            }

            bool pt_is_empty = TRUE;
            // Check all pt entries. If none are present, free the page table.
            for (int pt_ent = 0; pt_ent < PAGE_TABLE_ENTRIES; pt_ent++) {
                if (page_table[pt_ent] & PAGE_ENTRY_MASK_PRESENT) {
                    pt_is_empty = FALSE;
                    break;
                };
            }

            if (pt_is_empty) {
                free((uint8_t*)page_table);
                page_directory[pd_idx] = 0; // Remove from page directory
            }

        } else {
            return -1; // Page table not setup
        }
    }
    return 0;
}

uint32_t vmem_entry_get(uint32_t* page_directory, uint8_t* v_addr) {
    uint32_t v_addr_aligned = (uint32_t)v_addr & ~0xFFF;
    uint32_t pd_idx = v_addr_aligned / PAGE_SIZE / PAGE_TABLE_ENTRIES;
    uint32_t pt_idx = v_addr_aligned / PAGE_SIZE % PAGE_TABLE_ENTRIES;
    if (page_directory[pd_idx] & PAGE_ENTRY_MASK_PRESENT) {
        uint32_t* page_table = (uint32_t*)PADDR_TO_KADDR(page_directory[pd_idx] & 0xFFFFF000);
        return page_table[pt_idx];
    } else {
        return 0;
    }
}