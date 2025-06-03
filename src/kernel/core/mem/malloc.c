#include "malloc.h"
#include "core/mem/palloc.h"
#include "core/mem/memory.h"
#include "util/panic.h"
#include "util/logging.h"
#include "util/strutil.h"

// #define MALLOC_DBG

uint8_t* malloc(size_t size, int pid) {
    if (size == 0) {
        return 0;
    }
    __asm__("cli");
    // Get the process this request is for
    ProcessControlBlock* pcb = process_get(pid);
    
    // If this is the first call to malloc, initialize the heap
    if (!pcb->heap) {
        #ifdef MALLOC_DBG
        log_info("malloc: first call to malloc, initializing heap.");
        #endif
        malloc_init(pcb);
    }
    
    malloc_debug_blocks(pcb->heap);
    // Find a free block of memory
    Block* block = pcb->heap->start;
    uint8_t* addr = 0;    
    
    if (size <= pcb->heap->available) {
        addr = malloc_reserve_existing(size, pcb->heap);
    }

    
    while (addr == 0) {
        if (pcb->heap->arena_size * 2 > MALLOC_MAX_ARENA_SIZE) {
            // Out of memory, cannot double the arena size
            malloc_oom(__FUNCTION__);
        }
        #ifdef MALLOC_DBG
        log_number("malloc: doubling arena size to", pcb->heap->arena_size * 2, 16);
        #endif
        malloc_double_arena(pcb->heap);
        // Try to reserve again
        addr = malloc_reserve_existing(size, pcb->heap);
    }
    #ifdef MALLOC_DBG
    // log_number("malloc: reserved addr", (uint32_t)addr, 16);
    malloc_debug_blocks(pcb->heap);
    #endif
    __asm__("sti");

    return addr;
}

int mfree(uint8_t* addr, int pid) {
    if (addr == 0) {
        return -1;
    }
    __asm__("cli");
 // Get the process this request is for
    ProcessControlBlock* pcb = process_get(pid);
    
    // If this is the first call to malloc, initialize the heap
    if (!pcb->heap) {
        malloc_init(pcb);
    }
    
    // Find the block to free
    Block* block = pcb->heap->start;
    Block* prev = 0;
    while (block != 0) {
        if (block->addr == addr) {
            // Found the block to free
            block->used = false;
            pcb->heap->available += block->size;
            pcb->heap->num_blocks_reserved--;

            // Try to merge with next block
            if (block->next != 0 && !block->next->used && block->next->size == block->size) {
                malloc_merge_blocks(block, block->next, pcb->heap);

            }
            // Try to merge with previous block
            else if (prev != 0 && !prev->used && prev->size == block->size) {
                malloc_merge_blocks(prev, block, pcb->heap);
            }
            #ifdef MALLOC_DBG
            malloc_debug_blocks(pcb->heap);
            #endif
            __asm__("sti");
            return 0;
        }
        prev = block;
        block = block->next;
    }
    #ifdef MALLOC_DBG
    log_info("mfree: block not found");
    malloc_debug_blocks(pcb->heap);
    #endif
    __asm__("sti");

    return -1;
}

// Free all pages currently mapped with blocks. Done when a process exits.
void malloc_cleanup(Heap* heap) {
    if (heap == 0) {
        return; // Malloc wasn't initialized to begin with, no need to clean up
    }
    // Free all pages currently mapped with blocks
    // This is done when a process exits
    for (Block* block = heap->start; block != 0; block = block->next) {
        if (block->size != 0) {
            // Free the page
            pfree(block->addr, heap->page_directory);
        }
    }

    // Free the block pool pages
    BlockPool* block_pool = heap->bp_start;
    BlockPool* next = block_pool->next;
    while (block_pool != 0) {
        // Free the page
        pfree((uint8_t*)block_pool, heap->page_directory);
        block_pool = next;
        if (block_pool != 0) {
            next = block_pool->next;
        }
    }

    // Finally, free the heap page
    pfree((uint8_t*)heap, heap->page_directory);
}

// #define MALLOC_DBG_BLOCKS_LIST
#define MALLOC_DBG_BLOCKS_STAT

void malloc_debug_blocks(Heap* heap) {
    #ifdef MALLOC_DBG
    #ifdef MALLOC_DBG_BLOCKS_LIST
    Block* block = heap->start;
    while (block != 0) {
        print("malloc: block [");
        print_num((uint32_t)block->addr, 16); print("-");
        print_num((uint32_t)block->addr + block->size, 16);
        print("] desc [");
        print_num((uint32_t)block, 16);
        print("->");
        print_num((uint32_t)block->next, 16);
        if (block->used) {
            print("] used");
        } else {
            print("] free");
        }
        println("");
        block = block->next;
    }
    #endif
    #ifdef MALLOC_DBG_BLOCKS_STAT
    print("malloc: AVL= ");
    print_num(heap->available, 16);
    print(", ARN=");
    print_num(heap->arena_size, 16);
    print(", Blocks: M="); // Mapped blocks
    print_num(heap->num_blocks_in_use, 10);
    print(" R="); // Reserved blocks
    print_num(heap->num_blocks_reserved, 10);
    print(" T="); // Total blocks
    print_num(heap->num_blocks, 10);
    println("");
    #endif
    #endif
}

void malloc_double_arena(Heap* heap) {
    // Double the size of the malloc pool
    int pages_needed = heap->arena_size / PAGE_SIZE;

    malloc_alloc_pages(heap, pages_needed);
}


uint8_t* malloc_reserve_existing(size_t size, Heap* heap) {
    Block* block = heap->start;
    while (block != 0) {
        if (!block->used && block->size >= size) {
            // Found a free block
            Block* reserved = malloc_reserve_block(block, size, heap);
            heap->available -= reserved->size;
            reserved->used = true;
            heap->num_blocks_reserved++;
            return reserved->addr;
        }
        block = block->next;
    }
    return 0;
}

Block* malloc_reserve_block(Block* parent, size_t size, Heap* heap) {
    if (parent->size < size) {
        // Not enough space in the parent block
        return 0;
    }
    
    if (parent->size / 2 < size) {
        // Size too big to fit in a half block, return the parent block
        return parent;
    }
    
    // Split the parent block into two blocks
    malloc_split_block(parent, heap);
    Block* left = parent;
    Block* right = left->next;
    
    // Try reserving in the left block first
    Block* reserved_in_left = malloc_reserve_block(left, size, heap);
    if (reserved_in_left != 0) {
        return reserved_in_left;
    }
    // Try reserving in the right block
    Block* reserved_in_right = malloc_reserve_block(right, size, heap);
    if (reserved_in_right != 0) {
        return reserved_in_right;
    }

    return 0;
}

// Split a block of memory into two blocks
void malloc_split_block(Block* block, Heap* heap) {
    // Allocate a new Block struct for the right half
    Block* new_block = block_pool_alloc(heap);
    // Set the address and size of the new block
    // The new block will be half the size of the original block
    new_block->addr = block->addr + (block->size / 2);
    new_block->size = block->size / 2;
    new_block->used = false;
    new_block->next = block->next;
    // Set the address and size of the original block
    block->next = new_block;
    block->size = block->size / 2;

    heap->num_blocks_in_use++;
}

void malloc_merge_blocks(Block* left, Block* right, Heap* heap) {
    // Merge two blocks of memory into one
    left->size += right->size;
    left->next = right->next;

    block_mark_unused(right, heap);
}

void malloc_init(ProcessControlBlock* pcb) {
    // First call to malloc, initialize the heap
    uint8_t* info_page = palloc_next_free(pcb->page_directory);
    uint8_t* data_page = palloc_next_free(pcb->page_directory);

    if (info_page == 0 || data_page == 0) {
        malloc_oom(__FUNCTION__);
    }
    pcb->heap = (Heap*)info_page; // Use first few bytes of the first page to store Heap
    pcb->heap->page_directory = pcb->page_directory;
    pcb->heap->available = PAGE_SIZE;
    pcb->heap->arena_size = PAGE_SIZE;
    // Initialize first block - this will span the entire page

    block_pool_init(data_page, pcb->heap);
    pcb->heap->start = pcb->heap->bp_start->blocks;
    
}

void block_pool_init(uint8_t* data_start, Heap* heap) {
    // Initialize the block pool
    heap->bp_start = block_pool_new();
    
    // Initialize first block
    Block* first_block = block_pool_alloc(heap);

    first_block->addr = data_start;
    first_block->size = PAGE_SIZE;
    first_block->used = false;
    first_block->next = 0;

    // Update counters
    heap->num_blocks = BLOCK_POOL_CAPACITY;
    heap->num_blocks_in_use = 1; // First block descriptor is "in use" (not reserved yet, but allocated)
    heap->num_blocks_reserved = 0;
}


BlockPool* block_pool_new() {
    // Allocate a new block pool
    BlockPool* block_pool = (BlockPool*)palloc_next_free(current_page_directory);
    if (block_pool == 0) {
        malloc_oom(__FUNCTION__);
    }
    block_pool->next = 0;
    memfill(block_pool->blocks, BLOCK_POOL_CAPACITY * sizeof(Block), 0);

    return block_pool;
}

// Allocate a block within the block pool, and allocate new page if needed
// Does not set any data on the allocated block, which could have been reused
Block* block_pool_alloc(Heap* heap) {
    // 1. Try to find an already allocated block that isn't being used (size = 0)
    BlockPool* bp = heap->bp_start;
    while (true) {
        Block* block = bp->blocks;
        // Iterate over the blocks in the block pool
        for (size_t i = 0; i < BLOCK_POOL_CAPACITY; i++) {
            if (block->size == 0) {
                // Found a free block
                return block;
            }
            block++;
        }

        if (bp->next == 0) {
            // No more block pools to check
            break;
        }
        bp = bp->next;
    }
    
    // 2. If no free blocks are found, allocate a new page for the block pool
    // Allocate a new page
    BlockPool* new_pool = block_pool_new();
    bp->next = new_pool;

    Block* new_block = (Block*)&new_pool->blocks[0];
    // Update the block pool info
    heap->num_blocks += BLOCK_POOL_CAPACITY;
    return new_block;
}

void try_free_block_pool(BlockPool* block_pool, Heap* heap) {
    // Check if the block pool is empty
    Block* block = block_pool->blocks;
    for (size_t i = 0; i < BLOCK_POOL_CAPACITY; i++) {
        if (block->size != 0) {
            // Not empty, cannot free
            return;
        }
        block++;
    }
    // Free the block pool
    pfree((uint8_t*)block_pool, heap->page_directory);
    heap->num_blocks -= BLOCK_POOL_CAPACITY;
}

// For now, we don't dealloc block descriptors, just mark them as unused
void block_mark_unused(Block* block, Heap* heap) {
    block->size = 0;
    block->used = false;
    block->next = 0;
    heap->num_blocks_in_use--;

    BlockPool* block_pool = (BlockPool*)((uint32_t)block & ~0xFFF); // Get the block pool address (page aligned)
    try_free_block_pool(block_pool, heap);
}

void malloc_alloc_pages(Heap* heap, size_t num_pages) {
    uint8_t* start_addr = 0;
    for (size_t i = 0; i < num_pages; i++) {
        uint8_t* addr = palloc_next_free(heap->page_directory);
        
        if (addr == 0) {
            malloc_oom(__FUNCTION__);
        }

        if (start_addr == 0) {
            start_addr = addr;
        }
    }
    // Now setup a block for the newly allocated pages
    Block* last_block = malloc_get_last_block(heap->start);
    Block* new_block = (Block*)(last_block + sizeof(Block));
    new_block->addr = start_addr;
    new_block->size = num_pages * PAGE_SIZE;
    new_block->used = false;
    new_block->next = 0;
    last_block->next = new_block;

    // Update the heap  
    heap->arena_size += num_pages * PAGE_SIZE;
    heap->available += num_pages * PAGE_SIZE;
}

Block* malloc_get_last_block(Block* block) {
    while (block->next != 0) {
        block = block->next;
    }
    return block;
}

__attribute__((noreturn))
void malloc_oom(char* function) {
    // Out of memory handler

    char str[64] = "malloc: out of memory in ";
    strcat(str, function);
    strcat(str, "()");

    panic(str);
}