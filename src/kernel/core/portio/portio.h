/*
 * portio.h
 *
 * This header file defines functions for performing basic input/output operations
 * through hardware ports. It provides low-level routines for reading from and
 * writing to I/O ports, which is essential for interacting with hardware devices
 * in a kernel environment.
 */

#ifndef IO_H
#define IO_H

#include "types.h"

void outb(uint16_t port, uint8_t data);
void outw(uint16_t port, uint16_t data);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);

#endif