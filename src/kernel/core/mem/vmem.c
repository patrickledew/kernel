#include "vmem.h"
#include "memory.h"
#include "util/logging.h"
#include "offsets.h"

uint32_t k_page_directory[1024] __attribute__((aligned(4096)));
uint32_t k_page_table[1024] __attribute__((aligned(4096))); // 0x00000-0x1FFFF

void vmem_init() {
    // To test, we want to map a region of memory and try accessing it
    vmem_init_tables();
    vmem_enable();
    vmem_setup_stack();
    offset_kernel_set(0xc0000000);
    log_info("vmem_init: made it past vmem_enable");
    vmem_init_zap();
    log_info("vmem_init: made it past vmem_init_zap");

    while(1) {}
}

void vmem_init_tables() {
    log_info("vmem: creating page tables");
    // k_page_directory = (uint32_t*)alloc(4096);
    // k_page_table = (uint32_t*)alloc(4096);
    for (int i = 0; i < 1024; i++) {
        k_page_directory[i] = 0x2; // Set as not present, supervisor only, write enabled
    }

    // Map first 4MiB of memory for kernel use
    for (int i = 0; i < 1024; i++) {
        k_page_table[i] = (i * 0x1000) | PAGE_ENTRY_MASK_PRESENT | PAGE_ENTRY_MASK_READWRITE;
    }

    // Map virtual address ranges 0x0000-0x1FFFF and 0xc0000000-0xc000FFFF
    // to physical address range 0x0000-0x1FFFF
    k_page_directory[0] = (uint32_t)k_page_table | PAGE_ENTRY_MASK_PRESENT | PAGE_ENTRY_MASK_READWRITE;
    k_page_directory[0x300] = (uint32_t)k_page_table | PAGE_ENTRY_MASK_PRESENT | PAGE_ENTRY_MASK_READWRITE;
}

// Gets rid of the first (temporary) page directory entry mapping first 4MiB of virtual addresses
void vmem_init_zap() {
    k_page_directory[0] = 0;
    vmem_pd_load();
}

// Load the address of the kernel page directory into CR3
void vmem_pd_load() {
    // Since CR3 always refers to the absolute address of the page directory, and
    // we may be calling this function after paging is enabled (making the address
    // of k_page_directory 0xc0000000 bytes larger than it is in absolute terms),
    // we need to subtract the current virtual kernel offset from it.
    __asm__("mov %%eax, %%cr3" :: "a"((uint32_t)k_page_directory - (uint32_t)kernel_space_offset));
}