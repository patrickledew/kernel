// C Entry point for kernel. Invoked from kernel_init.s.
#include "core/interrupts.h"
#include "core/memory.h"
#include "util/logging.h"
#include "util/keyboard.h"
#include "util/timer.h"
#include "core/disk.h"
#include "core/fs.h"

int uptime = 0;
uint8_t* string = 0;

void show_uptime() {
    log_number_at("Uptime", uptime++, 10, 0, 55);
}

void trap() {
    while(1) {}
}

void print_interval() {
    print_char((*string++));
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

    log_info("kmain: Initializing disk driver...");
    disk_init();

    // After all interrupts have been registered, load IDT and enable interrupts
    log_info("kmain: enabling interrupts...");
    int_start();

    set_interval(1000, show_uptime);
    set_interval(10, update_cursor);

    log_info("kmain: Initializing FAT12 filesystem driver...");
    fs_init();

    set_color(0x0A);
    log_info("Done!");
    set_color(0x0F);

    // uint8_t* buf1 = alloc(PAGE_SIZE * 8);
    // uint8_t* buf2 = alloc(PAGE_SIZE * 8);

    // free_excess(buf1 + PAGE_SIZE * 8 - 100);

    trap();
}