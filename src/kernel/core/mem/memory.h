#ifndef MEMORY_H
#define MEMORY_H
#include "types.h"

void memcpy(uint8_t* src, uint8_t* dest, uint32_t size);
void memfill(uint8_t* buf, uint32_t size, uint8_t byte);

#endif