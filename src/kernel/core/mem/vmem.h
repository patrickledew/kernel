#ifndef VMEM_H
#define VMEM_H
#include "types.h"

// File containing code to manage virtual memory via page tables.

#define PAGE_ENTRY_MASK_PRESENT     0b000000000001 // Present
#define PAGE_ENTRY_MASK_READWRITE   0b000000000010 // 1 = read/write, 0 = read only
#define PAGE_ENTRY_MASK_USER        0b000000000100 // 1 = User, 0 = Supervisor
#define PAGE_ENTRY_MASK_WRITETHRU   0b000000001000 // Caching bs idk
#define PAGE_ENTRY_MASK_NOCACHE     0b000000010000 // Disable cache if set
#define PAGE_ENTRY_MASK_ACCESSED    0b000000100000 // Accessed bit
#define PAGE_ENTRY_MASK_PAGESIZE    0b000100000000 // Enable 4MiB paging if set
#define PAGE_ENTRY_MASK_UNUSED      0b111001000000 // Unused bits

extern uint32_t _KERNEL_PAGE_DIRECTORY[1024];
extern uint32_t _KERNEL_PAGE_TABLE[1024];


void vmem_init();
void vmem_load(uint32_t* page_directory);
void vmem_load_absolute(uint32_t* page_directory);
void vmem_zap_identity(); // Zaps the identity page table, which we will no longer use

// Map a virtual address to a physical address
void vmem_map(uint32_t* page_directory, uint8_t* v_addr, uint8_t* p_addr, uint32_t size);

#endif