#ifndef IO_H
#define IO_H

#define outb(port, byte) __asm__("out %%al, %%dx;" : : "a"(byte), "d"(port));
#define outw(port, byte) __asm__("outw %%ax, %%dx;" : : "a"(byte), "d"(port));

#define inb(port, var) __asm__("in %%dx, %%al" : "=a"(var) :  "d"(port));
#define inw(port, var) __asm__("inw %%dx, %%ax" : "=a"(var) :  "d"(port));

#endif