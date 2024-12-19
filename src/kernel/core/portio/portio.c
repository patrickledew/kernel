#include "portio.h"

void outb(uint16_t port, uint8_t data) {
    __asm__("out %%al, %%dx;" : : "a"(data), "d"(port));
}


void outw(uint16_t port, uint16_t data) {
    __asm__("outw %%ax, %%dx;" : : "a"(data), "d"(port));
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__("in %%dx, %%al" : "=a"(ret) :  "d"(port));
    return ret;
}

uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__("in %%dx, %%ax" : "=a"(ret) :  "d"(port));
    return ret;
}