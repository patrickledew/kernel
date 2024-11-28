ENTRY(_start)

MEMORY {
    KERNEL (rx) : ORIGIN = 0x10000, LENGTH = 0xFFFF
}
SECTIONS {
    .text : AT(0x0) {
        *(.kernel_init);
        *(.text);
    } >KERNEL
}