#include "malloc.test.h"
#include "util/logging.h"
#include "util/timer.h"

void malloc_test() {
    // Setup up "PCB" which just uses kernel address space
    processes[0].page_directory = current_page_directory;
    processes[0].malloc_info = 0;
    log_info("malloc_test: malloc test starting.");

    // Test malloc
    log_info("malloc_test: initialization.");
    uint8_t *addr;
    addr = malloc(0x1000, 0);
    assert_u32((uint32_t)addr, (uint32_t)processes[0].malloc_info->start->addr);
    assert_u32(0x1000, processes[0].malloc_info->start->size);
    assert_b(true, processes[0].malloc_info->start->used);
    assert_u32(BLOCK_POOL_CAPACITY, processes[0].malloc_info->num_blocks);

    
    log_info("malloc_test: can free single block");
    mfree(addr, 0);
    assert_b(false, processes[0].malloc_info->start->used);

    log_info("malloc_test: can allocate multiple blocks");
    uint8_t* a = malloc(0x1000 / 4, 0);
    uint8_t* b = malloc(0x1000 / 4, 0);
    uint8_t* c = malloc(0x1000 / 4, 0);
    uint8_t* d = malloc(0x1000 / 4, 0);
    assert_u32(4, processes[0].malloc_info->num_blocks_reserved);
    assert_u32(4, processes[0].malloc_info->num_blocks_in_use);
    assert_u32(0x1000 / 4, processes[0].malloc_info->start->size);
    assert_u32(0x1000 / 4, processes[0].malloc_info->start->next->size);
    assert_u32(0x1000 / 4, processes[0].malloc_info->start->next->next->size);
    assert_u32(0x1000 / 4, processes[0].malloc_info->start->next->next->next->size);
    assert_b(true, processes[0].malloc_info->start->used);
    assert_b(true, processes[0].malloc_info->start->next->used);
    assert_b(true, processes[0].malloc_info->start->next->next->used);
    assert_b(true, processes[0].malloc_info->start->next->next->next->used);

    log_info("malloc_test: can free blocks, merging behavior");
    mfree(c, 0); // 3rd block
    // Before freeing: |A|B|C|D|
    // After freeing:  |A|B|-|D|
    assert_b(false, processes[0].malloc_info->start->next->next->used);
    mfree(b, 0); // 2nd block
    // Before freeing: |A|B|-|D|
    // After freeing: |A| - |D|
    assert_b(false, processes[0].malloc_info->start->next->used);
    assert_u32(0x1000 / 2, processes[0].malloc_info->start->next->size);
    // Check that A->[-]->D
    assert_u32(processes[0].malloc_info->start->next->next->addr, (uint32_t)d);

    // Free remaining blocks
    mfree(a, 0);
    mfree(d, 0);

    // Check that all blocks are free
    assert_b(false, processes[0].malloc_info->start->used);
    assert_b(false, processes[0].malloc_info->start->next->used);
    assert_b(false, processes[0].malloc_info->start->next->next->used);
    
    assert_u32(0x1000, processes[0].malloc_info->available);
    assert_u32(0x1000, processes[0].malloc_info->arena_size);

    log_info("malloc_test: can allocate more than one page");
    uint8_t* e = malloc(0x1000 * 2, 0);
    // Current blocks: |-| - |-|
    // Target: |-| - |-|   E   .    E    |
    // E spans 2 pages, and so won't fit into the existing fragmented blocks.
    // Doubling once gives us one extra page, but E needs 2 pages, so it should double again.

    assert_u32(0x1000 * 4, processes[0].malloc_info->arena_size);

    mfree(e, 0);
    assert_u32(0x1000 * 4, processes[0].malloc_info->available);

    // Fragment memory a ton and see how we do when we free it (random)
    log_info("malloc_test: stress test");

    #define NUM_ALLOCATIONS 1000
    #define NUM_ITERS 1000
    #define MIN_SIZE 0x20
    #define MAX_SIZE 0x500

    log_number("Num. Allocations per Iteration", NUM_ALLOCATIONS, 10);
    log_number("Iterations", NUM_ITERS, 10);
    log_number("Min size", MIN_SIZE, 16);
    log_number("Max size", MAX_SIZE, 16);

    uint8_t* allocations[NUM_ALLOCATIONS];
    for(int i = 0; i < NUM_ITERS; i++) {
        // log_info("[ALLOCATING]");
        
        print("\r");
        print_num(i * NUM_ALLOCATIONS, 10);
        print(" / ");
        print_num(NUM_ITERS * NUM_ALLOCATIONS, 10);
        print(" (M=");
        print_num(processes[0].malloc_info->num_blocks_in_use, 10);
        print(" R=");
        print_num(processes[0].malloc_info->num_blocks_reserved, 10);
        print(" T=");
        print_num(processes[0].malloc_info->num_blocks, 10);
        print(") - Allocating                            ");

        for(int j = 0; j < NUM_ALLOCATIONS; j++) {
            uint32_t tick;
            asm volatile("rdtsc" : "=A"(tick));

            uint32_t size = (tick % (MAX_SIZE-MIN_SIZE)) + MIN_SIZE;
            // log_number("malloc_test: allocating size", size, 16);
            allocations[j] = malloc(size, 0);
            // log_number("alloced", allocations[j], 16);
        }
        // Verify the available space is less than the arena size minus the size of the blocks
        size_t total_size = 0;
        for (Block* block = processes[0].malloc_info->start; block != 0; block = block->next) {
            if (block->used) {
                total_size += block->size;
            }
        }

        assert_u32(processes[0].malloc_info->arena_size - total_size, processes[0].malloc_info->available);
        

        print("\r");
        print_num(i * NUM_ALLOCATIONS, 10);
        print(" / ");
        print_num(NUM_ITERS * NUM_ALLOCATIONS, 10);
        print(" (M=");
        print_num(processes[0].malloc_info->num_blocks_in_use, 10);
        print(" R=");
        print_num(processes[0].malloc_info->num_blocks_reserved, 10);
        print(" T=");
        print_num(processes[0].malloc_info->num_blocks, 10);
        print(") - Freeing                            ");

        // log_info("[FREEING]");
        for(int j = 0; j < NUM_ALLOCATIONS; j++) {
            // log_number("freeing", allocations[j], 16);
            mfree(allocations[j], 0);
        }

        // Verify everything is free
        for (Block* block = processes[0].malloc_info->start; block != 0; block = block->next) {
            assert_b(false, block->used);
        }
    }
    println("");

    // Cleanup
    malloc_cleanup(processes[0].malloc_info);

    // malloc_debug_blocks(processes[0].malloc_info);

    log_info("malloc_test: malloc test ended.");

}