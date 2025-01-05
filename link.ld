ENTRY(_start)

MEMORY {
    KERNEL (rx) : ORIGIN = 0x100000, LENGTH = 0xFFFF
}
SECTIONS {
    .kernel : AT(0x0) {
        __KERNEL_BEGIN = .;
        *(.kernel_init);
        *(.text .text.*);
        *(.data .data.*);
        *(.rodata);
        *(.bss);
        *(.eh_frame);
        *(.got.plt);
         __KERNEL_END = .;
    } >KERNEL
}