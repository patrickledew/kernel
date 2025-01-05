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

extern void vmem_enable();
extern void vmem_setup_stack();

void vmem_init();

void vmem_init_tables();

void vmem_init_zap();

void vmem_pd_load();

// Map a virtual address to a physical address
void vmem_map(uint32_t* page_directory, uint8_t* v_addr, uint8_t* p_addr, uint32_t size);

#endif