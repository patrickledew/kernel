/* malloc.h
 *
 * This header file defines the interface for a user-space memory allocator,
 * providing malloc functionality for processes. It implements
 * a block-based memory management system that can grow dynamically by
 * requesting additional pages from the kernel. The allocator manages memory
 * chunks with metadata blocks and supports operations like splitting and
 * merging to minimize fragmentation.
 */
#ifndef MALLOC_H
#define MALLOC_H

#include <stddef.h>
#include <stdbool.h>
#include "types.h"
#include "core/mem/vmem.h"

typedef struct MallocInfo MallocInfo;

#include "core/proc/proc.h"

// User space memory allocator
// Unlike the other allocators (alloc.c, palloc.c), this allocator is for allocating
// specific amounts of memory (down to the byte) in user space programs. For this we
// make use of palloc and the PID of the process to allocate more memory to the process
// when requested.

#define MALLOC_MAX_ARENA_SIZE 0xB0000000

// Information about a process's heap

struct Block;
struct BlockPool;


// Each block is a chunk of memory with a defined start address and size.
// The block also contains a pointer to the next block in the list and a flag
// indicating whether the block is in use or free.
// The blocks are linked together in a singly linked list, with the start
// pointer of the first block stored in the MallocInfo structure.
typedef struct Block {
    uint8_t* addr;
    size_t size;
    bool used;
    struct Block* next;
} Block;

// A page-sized section of memory used to store metadata about the blocks.
// This is part of a linked list, so that if one page runs out of space to hold
// all the different block descriptors, we can allocate a new page

#define BLOCK_POOL_CAPACITY ((PAGE_SIZE - sizeof(uint8_t*)) / sizeof(Block))
typedef struct BlockPool {
    struct BlockPool* next; // Pointer to the next block pool
    Block blocks[BLOCK_POOL_CAPACITY]
} BlockPool;

typedef struct MallocInfo {
    uint32_t* page_directory; // Pointer to the process's page directory
    Block* start; // Pointer to the first block in the arena
    BlockPool* bp_start; // Pointer to the first page in the block pool
    size_t available; // Amount of memory available for allocation
    size_t arena_size; // Total size of the memory arena
    size_t num_blocks; // Number of blocks in the block pool
    size_t num_blocks_in_use; // Number of blocks in the block pool mapped to regions
    size_t num_blocks_reserved; // Number of blocks reserved for use (not "free")
} MallocInfo;



uint8_t* malloc(size_t size, int pid);
int mfree(uint8_t* addr, int pid);


// Initializes the heap by populating the ProcessControlBlock's malloc_info structure.
void malloc_init(ProcessControlBlock* pcb);
void malloc_cleanup(MallocInfo* malloc_info);

// Allocates a set number of pages and adds a block that spans those pages.
void malloc_alloc_pages(MallocInfo* malloc_info, size_t num_pages);

// Doubles the size of the malloc arena by allocating more pages. Only called when the process needs more memory than is currently available.
void malloc_double_arena(MallocInfo* malloc_info);

// Gets the last block in the linked list of blocks.
Block* malloc_get_last_block(Block* block);

// Attempts to find an existing block of memory that can accommodate the requested size.
uint8_t* malloc_reserve_existing(size_t size, MallocInfo* malloc_info);

// Reserves a block of memory from the existing block pool by recursively subdividing blocks
// until the smallest size that can accommodate the requested size is found.
Block* malloc_reserve_block(Block* block, size_t size, MallocInfo* malloc_info);

// Splits a block into two smaller blocks of equal size.
// The provided block will half its size split in half, and block->next will point to the second block.
void malloc_split_block(Block* block, MallocInfo* malloc_info);

// Merges two adjacent blocks of the same size into one larger block.
// Left will become the merged block, and the right block will be marked as unused.
void malloc_merge_blocks(Block* left, Block* right, MallocInfo* malloc_info);

// Handles out-of-memory situations by logging an error and halting the process.
void malloc_oom(char* function);

// Debugging function to print the current state of the malloc blocks.
void malloc_debug_blocks(MallocInfo* malloc_info);

// Functions to manage the block pool (metadata about each block)

// Initializes the block pool to be immediately after the MallocInfo structure.
// Also sets up the first block to span the entire arena size.
void block_pool_init(uint8_t* data_start, MallocInfo* malloc_info);

// Creates or reuses a block descriptor and links it to the block pool.
// Allocates a new page if necessary to store the block descriptor.
Block* block_pool_alloc(MallocInfo* malloc_info);

// Marks a block descriptor as unused, so it can later be used to store
// information about a different section of memory.
void block_mark_unused(Block* block, MallocInfo* malloc_info);
BlockPool* block_pool_new();

#endif