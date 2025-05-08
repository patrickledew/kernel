#ifndef MALLOC_H
#define MALLOC_H

#include <stddef.h>
#include <stdbool.h>
#include "types.h"

typedef struct MallocInfo MallocInfo;

#include "core/proc/proc.h"

// User space memory allocator
// Unlike the other allocators (alloc.c, palloc.c), this allocator is for allocating
// specific amounts of memory (down to the byte) in user space programs. For this we
// make use of palloc and the PID of the process to allocate more memory to the process
// when requested.

#define MALLOC_MAX_ARENA_SIZE 0x4000 // 4 pages (2 doublings)

// Information about a process's heap

struct Block;

typedef struct Block {
    uint8_t* addr;
    size_t size;
    bool used;
    struct Block* next;
} Block;

typedef struct MallocInfo {
    Block* start;
    size_t available;
    size_t arena_size;
} MallocInfo;

uint8_t* malloc_impl(size_t size, int pid);


void malloc_init(ProcessControlBlock* pcb);
void malloc_alloc_pages(ProcessControlBlock* pcb, size_t num_pages);
void malloc_double_arena(ProcessControlBlock* pcb);
Block* malloc_get_last_block(Block* block);
uint8_t* malloc_reserve_existing(size_t size, MallocInfo* malloc_info);
Block* malloc_reserve_block(Block* block, size_t size);
void malloc_split_block(Block* block);
void malloc_oom();

#endif