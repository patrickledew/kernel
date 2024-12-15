#ifndef IO_H
#define IO_H

#include "types.h"

void outb(uint16_t port, uint8_t data);
void outw(uint16_t port, uint16_t data);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);

#endif