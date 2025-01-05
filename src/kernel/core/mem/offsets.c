#include "offsets.h"

// Kernel space is initially set to 0x0000-4MiB,
// but when vmem is enabled, we make it begin at 0xc0000000

void offset_kernel_set(uint8_t* offset) {
    kernel_space_offset = offset;
}