#include "memory.h"

void memcpy(uint8_t* src, uint8_t* dest, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++) {
        *(dest + i) = *(src + i);
    }
}

void memfill(uint8_t* buf, uint32_t size, uint8_t byte) {
    for (uint32_t i = 0; i < size; i++) {
        buf[i] = byte;
    }
}