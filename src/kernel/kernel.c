#include "core/mem/vmem.h"
#include "core/mem/alloc.h"
#include "core/mem/memory.h"
#include "core/interrupts/interrupts.h"
#include "core/vga/vga.h"
#include "core/disk/disk.h"
#include "core/fs/fat/fat12.h"
#include "core/fs/fs.h"
#include "core/gdt/gdt.h"
#include "core/proc/loader.h"
#include "core/interrupts/syscall.h"
// #include "core/proc/proc.h"

#include "util/timer.h"
#include "util/assert.h"
#include "util/logging.h"
#include "util/keyboard.h"
#include "util/strutil.h"

#include "test/fs.test.h"
#include "test/vmem.test.h"
#include "test/elf.test.h"
#include "test/palloc.test.h"

int uptime = 0;

void show_uptime() {
    log_number_at_u("UP", uptime++, 10, 0, 55);
}

void trap() {
    while(1) {}
}

void print_kernel_addr() {    
    print_screen_fill(' ', 0x0F);
    uint8_t c = print_color_get();
    print_color_set(0x0A);
    print("kmain: kernel virt addr: 0x");
    print_num_u((uint32_t)KERNEL_BEGIN, 16);
    print("-0x");
    print_num_u((uint32_t)KERNEL_END, 16);
    println(".");
    print_color_set(c);
}

// C entry point for kernel. Invoked from kernel_init.s.
void kmain() {
    print_kernel_addr();

    gdt_init();

    /** Finish setting up paging */
    vmem_init();

    /** Setup kernel page allocator */
    alloc_init(0x1000);
    /** Setup process memory allocator */
    palloc_init();

    /** Initialize interrupts, then initialize everything that registers an interrupt handler */
    int_init(); // Initialize interrupt descriptor table (IDT)
    
    timer_init(1000); // Initialize hardware timer at 1khz
    timer_interval_set(1000, show_uptime);
    timer_interval_set(10, print_cursor_refresh);

    keyboard_init(); // Initialize keyboard driver
    disk_init(); // Initialize ATA disk driver
    syscall_init();
    // Once all ISRs are registered, enable interrupts
    int_start();

    /** Initialize the filesystem driver to read in our filesystem */
    fat_init();

    /**
     * -=-=-=-=-=-=-=-=-=-=-=-=-=-=-
     * Extra space to do other stuff.
     * -=-=-=-=-=-=-=-=-=-=-=-=-=-=-
     */
    // fs_test();
    // vmem_test();
    // elf_test();
    // loader_test("/HELLO.ELF");
    fat_dbg_print_rootdir();
    exec("/HELLO.ELF");
    // palloc_test();
    /**
     * -=-=-=-=-=-=-=-=-=-=-=-=-=-=-
     */

    /** Trap the kernel in an infinite loop */
    trap();
}