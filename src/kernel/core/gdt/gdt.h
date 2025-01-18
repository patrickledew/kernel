#ifndef GDT_H
#define GDT_H

#include "types.h"

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t flags_limit;
    uint8_t base_high;
} __attribute__((packed)) gdt_segment_descriptor;

typedef struct {
    uint16_t size;
    gdt_segment_descriptor* offset;
} __attribute__((packed)) gdt_descriptor;

void gdt_init();
void gdt_load(gdt_descriptor* gdtr);

#endif