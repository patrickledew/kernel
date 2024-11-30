// C Entry point for kernel. Invoked from kernel_init.s.
#include "util/logging.h"
#include "core/interrupts.h"

void kmain() {
    fill_screen(' ', 0x00);
    log_info("kmain: initializing IDT...");
    init_idt(); // Initialize interrupt descriptor table (IDT)
    
    // induce GP fault
    // uint32_t x = 0xFFFFFFFF;
    // __asm__("mov %0, %%cr4" : : "a"(x));

    char c = 'A';
    print_char_at(c++, 0x0D, VIDEO_ROWS-1, VIDEO_COLS-1);

    while(1) {
        print_char_at(c++, 0x0D, VIDEO_ROWS-1, VIDEO_COLS-1);
    }
}
