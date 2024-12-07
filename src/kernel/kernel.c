// C Entry point for kernel. Invoked from kernel_init.s.
#include "core/interrupts.h"
#include "core/memory.h"
#include "util/logging.h"
#include "util/keyboard.h"
#include "util/timer.h"

void test_routine() {
    log_info("This is a test interval");
}

void kmain() {
    fill_screen(' ', 0x0F);
    log_number("kmain: kernel begin", (uint32_t)kernel_begin, 16);
    log_number("kmain: kernel end", (uint32_t)kernel_end, 16);

    log_info("kmain: initializing physical memory manager...");
    mem_init(0x1000);

    log_info("kmain: initializing interrupt descriptor table...");
    init_idt(); // Initialize interrupt descriptor table (IDT)

    log_info("kmain: initializing hardware timer...");
    init_pit(1000); // Initialize hardware timer at 1khz

    log_info("kmain: initializing keyboard...");
    keyboard_init(); // Initialize keyboard driver

    // After all interrupts have been registered, load IDT and enable interrupts
    log_info("kmain: enabling interrupts...");
    int_start();

    set_interval(500, test_routine);
    set_interval(10, update_cursor);

    set_color(0x0A);
    log_info("Done!");
    set_color(0x0F);

    uint8_t* allocated_A = alloc(256); // Should allocate to page 1
    log_number("Allocated A at", (uint32_t)allocated_A, 16);
    print_num_x(*(uint32_t*)kernel_end, 2, TRUE, 16);
    println("");
    uint8_t* allocated_B = alloc(5126); // Should allocate to page 2 and 3
    log_number("Allocated B at", (uint32_t)allocated_B, 16);
    print_num_x(*(uint32_t*)kernel_end, 2, TRUE, 16);
    println("");
    uint8_t* allocated_C = alloc(128); // Should allocate to page 4
    log_number("Allocated C at", (uint32_t)allocated_C, 16);
    print_num_x(*(uint32_t*)kernel_end, 2, TRUE, 16);
    println("");
    free(allocated_B); // Free the 2-pager one
    print_num_x(*(uint32_t*)kernel_end, 2, TRUE, 16);
    println("");
    
    uint8_t* allocated_D = alloc(4096); // Should allocate exactly 1 page to 2
    log_number("Allocated D at", (uint32_t)allocated_D, 16);
    print_num_x(*(uint32_t*)kernel_end, 2, TRUE, 16);
    println("");
    uint8_t* allocated_E = alloc(1024); // Page 3
    log_number("Allocated E at", (uint32_t)allocated_E, 16);
    print_num_x(*(uint32_t*)kernel_end, 2, TRUE, 16);
    println("");
    uint8_t* allocated_F = alloc(512); // Even though this COULD fit in one page, due to page size it will not and be placed in 5
    log_number("Allocated F at", (uint32_t)allocated_F, 16);
    print_num_x(*(uint32_t*)kernel_end, 2, TRUE, 16);
    println("");
    

    while(1) {
    }
}
