ENTRY(_start)

MEMORY {
    KERNEL (rx) : ORIGIN = 0x10000, LENGTH = 0xFFFF
}
SECTIONS {
    .kernel : AT(0x0) {
        __KERNEL_BEGIN = .;
        *(.kernel_init);
        *(.text);
        *(.data);
        *(.rodata);
        *(.bss);
        *(*);
         __KERNEL_END = .;
    } >KERNEL
}