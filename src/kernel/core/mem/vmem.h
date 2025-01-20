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
#define PAGE_ENTRY_MASK_PALLOC_END  0b001000000000 // End of a palloc block
#define PAGE_ENTRY_MASK_UNUSED      0b110001000000 // Unused bits

#define PAGE_SIZE 0x1000
#define PAGE_TABLE_ENTRIES 0x400


// Conversions from a variable address (in kernel space) to a physical address and back
#define KADDR_TO_PADDR(kaddr) ((uint8_t*)((uint32_t)(kaddr) - 0xC0000000 + 0x100000))
#define PADDR_TO_KADDR(paddr) ((uint8_t*)((uint32_t)(paddr) + 0xC0000000 - 0x100000))

extern uint32_t _KERNEL_PAGE_DIRECTORY[PAGE_TABLE_ENTRIES]; // each entry corresponds to 0x400 pages
extern uint32_t _KERNEL_PAGE_TABLE[PAGE_TABLE_ENTRIES]; // each entry corresponds to 1 page (0x1000 bytes)

void vmem_init();
void vmem_load(uint32_t* page_directory);
void vmem_load_absolute(uint32_t* page_directory);
void vmem_zap_identity(); // Zaps the identity page table, which we will no longer use

uint32_t* vmem_pd_create();
void vmem_pd_destroy();

// Map a virtual address to a physical address
int vmem_map(uint32_t* page_directory, uint8_t* p_addr, uint8_t* v_addr, uint32_t size, uint16_t flags);
int vmem_unmap(uint32_t* page_directory, uint8_t* v_addr, uint32_t num_pages);

uint32_t vmem_entry_get(uint32_t* page_directory, uint8_t* v_addr);
#endif