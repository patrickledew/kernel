// C Entry point for kernel. Invoked from kernel_init.s.
#include "print.h"
#include "interrupts.h"
void fibb(int n) {
    int a = 1;
    int b = 1;

    for (int i = 0; i < n; i++) {
        set_cursor_pos(i, 40);
        print_num(a, 10, 0x0F);
        int t = b;
        b = a + b;
        a = t;
    }
}

void kmain() {
    fill_screen(' ', 0x00);
    print("kmain: initializing IDT...", 0x0F);
    init_idt(); // Initialize interrupt descriptor table (IDT)
    // print("Hello world! This is my new kernel!\n", 0x0D);
    // fibb(VIDEO_ROWS);
    // __asm__ ("mov %eax, 0");
    // __asm__ ("div 0");
    while(1) {}
}
