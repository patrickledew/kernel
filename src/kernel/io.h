#ifndef IO_H
#define IO_H

#define outb(port, byte) __asm__("out %%al, %%dx" : : "a"(byte), "d"(port));
#define inb(port, byte) __asm__("in %%al, %%dx" : : "a"(byte), "d"(port));

#endif