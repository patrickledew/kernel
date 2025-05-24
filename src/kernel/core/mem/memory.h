/*
 * memory.h
 *
 * This header file provides essential memory manipulation functions for the kernel.
 * It includes basic operations such as memory copy and memory fill, which are
 * fundamental for managing system memory and data movement operations within
 * the kernel space.
 */

#ifndef MEMORY_H
#define MEMORY_H
#include "types.h"

void memcpy(uint8_t* src, uint8_t* dest, uint32_t size);
void memfill(uint8_t* buf, uint32_t size, uint8_t byte);

#endif