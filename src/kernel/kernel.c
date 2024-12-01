// C Entry point for kernel. Invoked from kernel_init.s.
#include "util/logging.h"
#include "core/interrupts.h"
#include "util/keyboard.h"

void kmain() {
    fill_screen(' ', 0x0F);

    log_info("kmain: initializing interrupts...");
    init_idt(); // Initialize interrupt descriptor table (IDT)
    log_info("kmain: initializing keyboard...");
    keyboard_init(); // Initialize keyboard driver

    while(1) {
    }
}
