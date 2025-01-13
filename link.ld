ENTRY(_start) /* Located in kernel_init.s */

SECTIONS {
    /* Our kernel initialization code will be jumped to before we switch on virtual memory,
       so we use base address 0x100000, which is where the kernel is in physical memory. */
    .kernel_init 0x100000 : AT(0) {
        *(.kernel_init);
    }

    /* The actual kmain function as well as the rest of the kernel will be invoked after virtual memory
       is enabled, so we use 0xC0000000 */
    .kernel (0xC0000000 + SIZEOF(.kernel_init)) : AT(SIZEOF(.kernel_init)) {
        __KERNEL_BEGIN = .;
        *(.text .text.*);
        *(.data .data.*);
        *(.rodata);
        *(.bss);
        *(.eh_frame);
        *(.got.plt);
        . = ALIGN(4096);
         __KERNEL_END = .;
    }
}