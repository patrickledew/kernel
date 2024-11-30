// C Entry point for kernel. Invoked from kernel_init.s.
#include "util/logging.h"
#include "core/interrupts.h"
#include "util/keyboard.h"
void kmain() {
    fill_screen(' ', 0x00);
    log_info("kmain: initializing IDT...");
    init_idt(); // Initialize interrupt descriptor table (IDT)
    keyboard_init(); // Initialize keyboard driver

    char c = 'A';
    while(1) {
        print_char_at(c++, 0x0D, VIDEO_ROWS-1, VIDEO_COLS-1);
    }
}
