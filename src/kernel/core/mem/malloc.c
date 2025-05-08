#include "malloc.h"
#include "core/mem/palloc.h"
#include "core/mem/memory.h"
#include "util/panic.h"
#include "util/logging.h"

uint8_t* malloc_impl(size_t size, int pid) {
    // Get the process this request is for
    ProcessControlBlock* pcb = process_get(pid);
    log_number("malloc_impl: 1 malloc info addr", (uint32_t)pcb->malloc_info, 16);
    
    // If this is the first call to malloc, initialize the malloc info
    if (!pcb->malloc_info) {
        log_info("malloc_impl: first call to malloc, initializing malloc info.");
        malloc_init(pcb);
        log_number("malloc_impl: 2 malloc info addr", (uint32_t)pcb->malloc_info, 16);

    }

    
    log_number("malloc_impl: orig available", pcb->malloc_info->available, 16);
    log_number("malloc_impl: orig arena size", pcb->malloc_info->arena_size, 16);
    // Find a free block of memory
    Block* block = pcb->malloc_info->start;
    uint8_t* addr = 0;
    
    
    if (size <= pcb->malloc_info->available) {
        log_number("malloc_impl: reserving from existing arena w/ size", pcb->malloc_info->arena_size, 16);
        addr = malloc_reserve_existing(size, pcb->malloc_info);
    }

    
    while (addr == 0) {
        if (pcb->malloc_info->arena_size * 2 > MALLOC_MAX_ARENA_SIZE) {
            // Out of memory, cannot double the arena size
            malloc_oom();
        }
        log_number("malloc_impl: doubling arena size to", pcb->malloc_info->arena_size * 2, 16);

        malloc_double_arena(pcb);
        // Try to reserve again
        log_number("malloc_impl: reserving from existing arena w/ size", pcb->malloc_info->arena_size, 16);
        addr = malloc_reserve_existing(size, pcb->malloc_info);
    }

    log_number("malloc_impl: reserved addr", (uint32_t)addr, 16);
    log_number("malloc_impl: new available", pcb->malloc_info->available, 16);
    log_number("malloc_impl: new arena size", pcb->malloc_info->arena_size, 16);
    return addr;
    // No free block found, allocate new pages

}

void malloc_double_arena(ProcessControlBlock* pcb) {
    // Double the size of the malloc pool
    int pages_needed = pcb->malloc_info->arena_size / PAGE_SIZE;

    malloc_alloc_pages(pcb, pages_needed);
}

uint8_t* malloc_reserve_existing(size_t size, MallocInfo* malloc_info) {
    Block* block = malloc_info->start;
    while (block != 0) {
        if (!block->used && block->size >= size) {
            // Found a free block
            Block* reserved = malloc_reserve_block(block, size);
            malloc_info->available -= reserved->size;
            return reserved->addr;
        }
        block = block->next;
    }
    return 0;
}

Block* malloc_reserve_block(Block* parent, size_t size) {
    if (parent->size < size) {
        // Not enough space in the parent block
        return 0;
    }
    
    if (parent->size / 2 < size) {
        // Size too big to fit in a half block, return the parent block
        parent->used = true;
        return parent;
    }
    
    // Split the parent block into two blocks
    malloc_split_block(parent);
    Block* left = parent;
    Block* right = left->next;
    
    // Try reserving in the left block first
    Block* reserved_in_left = malloc_reserve_block(left, size);
    if (reserved_in_left != 0) {
        return reserved_in_left;
    }
    // Try reserving in the right block
    Block* reserved_in_right = malloc_reserve_block(right, size);
    if (reserved_in_right != 0) {
        return reserved_in_right;
    }

    return 0;
}

// Split a block of memory into two blocks
void malloc_split_block(Block* block) {
    // Allocate a new Block struct for the right half
    Block* new_block = (Block*)((uint8_t*)block + sizeof(Block));
    // Set the address and size of the new block
    // The new block will be half the size of the original block
    new_block->addr = block->addr + (block->size / 2);
    new_block->size = block->size / 2;
    new_block->used = false;
    new_block->next = block->next;
    // Set the address and size of the original block
    block->next = new_block;
    block->size = block->size / 2;
}

void malloc_init(ProcessControlBlock* pcb) {
    // First call to malloc, initialize the malloc info
    uint8_t* info_page = palloc_next_free(pcb->page_directory);
    uint8_t* data_page = palloc_next_free(pcb->page_directory);

    if (info_page == 0 || data_page == 0) {
        malloc_oom();
    }
    pcb->malloc_info = (MallocInfo*)info_page; // Use first few bytes of the first page to store MallocInfo
    pcb->malloc_info->available = PAGE_SIZE;
    pcb->malloc_info->arena_size = PAGE_SIZE;
    // Initialize first block - this will span the entire page
    pcb->malloc_info->start = (Block*)(info_page + sizeof(MallocInfo));
    pcb->malloc_info->start->addr = data_page;
    pcb->malloc_info->start->used = false;
    pcb->malloc_info->start->size = PAGE_SIZE;
    pcb->malloc_info->start->next = 0;
}

void malloc_alloc_pages(ProcessControlBlock* pcb, size_t num_pages) {
    uint8_t* start_addr = 0;
    for (size_t i = 0; i < num_pages; i++) {
        uint8_t* addr = palloc_next_free(pcb->page_directory);
        
        if (addr == 0) {
            malloc_oom();
        }

        if (start_addr == 0) {
            start_addr = addr;
        }
    }
    // Now setup a block for the newly allocated pages
    Block* last_block = malloc_get_last_block(pcb->malloc_info->start);
    Block* new_block = (Block*)(last_block + sizeof(Block));
    new_block->addr = start_addr;
    new_block->size = num_pages * PAGE_SIZE;
    new_block->used = false;
    new_block->next = 0;
    last_block->next = new_block;

    // Update the malloc info  
    pcb->malloc_info->arena_size += num_pages * PAGE_SIZE;
    pcb->malloc_info->available += num_pages * PAGE_SIZE;

}

Block* malloc_get_last_block(Block* block) {
    while (block->next != 0) {
        block = block->next;
    }
    return block;
}

__attribute__((noreturn))
void malloc_oom() {
    // Out of memory handler
    panic("malloc: out of memory");
}