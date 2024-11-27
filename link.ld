ENTRY(_start)

MEMORY {
    KERNEL (rx) : ORIGIN = 0x10000, LENGTH = 0x1000
}
SECTIONS {
    .text : AT(0x0) {
        *(.text);
    } >KERNEL
}