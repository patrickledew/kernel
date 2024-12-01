#ifndef IO_H
#define IO_H

#define outb(port, byte) __asm__("out %%al, %%dx;" : : "a"(byte), "d"(port));

#define inb(port, var) __asm__("in %%dx, %%al" : "=a"(var) :  "d"(port));

#endif