// C Entry point for kernel. Invoked from kernel_init.s.
#include "core/interrupts/interrupts.h"
#include "core/mem/memory.h"
#include "util/logging.h"
#include "util/keyboard.h"
#include "util/timer.h"
#include "core/vga/vga.h"
#include "core/disk/disk.h"
#include "core/fs/fat/fat12.h"

int uptime = 0;

void show_uptime() {
    log_number_at("Uptime", uptime++, 10, 0, 55);
}

void trap() {
    while(1) {}
}

void kmain() {
    print_screen_fill(' ', 0x0F);
    log_number("kmain: kernel begin", (uint32_t)__KERNEL_BEGIN, 16);
    log_number("kmain: kernel end", (uint32_t)__KERNEL_END, 16);

    log_info("kmain: initializing physical memory manager...");
    mem_init(0x1000);

    log_info("kmain: initializing interrupt descriptor table...");
    int_idt_setup(); // Initialize interrupt descriptor table (IDT)

    log_info("kmain: initializing hardware timer...");
    timer_pit_init(1000); // Initialize hardware timer at 1khz

    log_info("kmain: initializing keyboard...");
    keyboard_init(); // Initialize keyboard driver

    log_info("kmain: Initializing disk driver...");
    disk_init();

    // After all interrupts have been registered, load IDT and enable interrupts
    log_info("kmain: enabling interrupts...");
    int_start();

    timer_interval_set(1000, show_uptime);
    timer_interval_set(10, print_cursor_refresh);

    log_info("kmain: Initializing FAT12 filesystem driver...");
    fat_init();

    print_color_set(0x0A);
    log_info("Done!");
    print_color_set(0x0F);

    // uint8_t* buf1 = alloc(PAGE_SIZE * 8);
    // uint8_t* buf2 = alloc(PAGE_SIZE * 8);

    // free_excess(buf1 + PAGE_SIZE * 8 - 100);

    trap();
}