#ifndef OFFSETS_H
#define OFFSETS_H

#include "types.h"

// Offset to be used when writing to the kernel address space.
// This gets updated to 0xC0000000 when virtual memory is enabled.
extern uint8_t* kernel_space_offset;

void offset_kernel_set(uint8_t* offset);

#endif