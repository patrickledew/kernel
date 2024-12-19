#ifndef MEMORY_H
#define MEMORY_H
/*
* Physical memory manager
*/

#include "types.h"

#define PAGE_SIZE 0x1000

#define BYTE_IDX(page_idx) (((page_idx) * 2) / 8)
#define BIT_IDX(page_idx) (((page_idx) * 2) % 8)

#define PAGE_USED(page_idx) (bool)(p_bitmap[BYTE_IDX(page_idx)] >> BIT_IDX(page_idx) & 0x01)
#define PAGE_END(page_idx) (bool)(p_bitmap[BYTE_IDX(page_idx)] >> BIT_IDX(page_idx) & 0x10)

#define SET_PAGE_USED(page_idx) p_bitmap[BYTE_IDX(page_idx)] |= (0b01 << BIT_IDX(page_idx));
#define SET_PAGE_END(page_idx) p_bitmap[BYTE_IDX(page_idx)] |= (0b10 << BIT_IDX(page_idx));
                     

// Need to know the address where kernel is loaded, and how big it is

// These are symbols introduced by the linker. They are located at the beginning and end of the kernel image.
extern uint8_t* __KERNEL_BEGIN;
extern uint8_t* __KERNEL_END;

void mem_init(int pages);
uint8_t* alloc(uint32_t size);
uint32_t free(uint8_t* addr);
uint32_t free_excess(uint8_t* addr);

void memcpy(uint8_t* src, uint8_t* dest, uint32_t size);

#endif