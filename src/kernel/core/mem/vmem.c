#include "vmem.h"
#include "memory.h"
#include "util/logging.h"

uint32_t* kernel_page_directory;
uint32_t* kernel_page_table;

void vmem_init() {
    kernel_page_directory = _KERNEL_PAGE_DIRECTORY + 0xC0000000 / 4;
    kernel_page_table = _KERNEL_PAGE_TABLE + 0xC0000000 / 4;
    log_info("vmem_init: cleaning up memory map.");
    vmem_zap_identity();
}

void vmem_zap_identity() {
     // Overwrite first page directory entry
     // this is the identity entry that maps 0x0 - 0x400000 to itself
    kernel_page_directory[0] = 0;
    vmem_load(kernel_page_directory);
}

void vmem_load(uint32_t* page_directory) {
    vmem_load_absolute(page_directory - 0xC0000000 / 4);
}

void vmem_load_absolute(uint32_t* page_directory) {
    __asm__("mov %0, %%cr3" :: "a"(page_directory));
}