/*
 * gdt.h
 *
 * Header file defining the structures and functions for managing the Global Descriptor
 * Table (GDT). The GDT is a data structure used by x86 processors for memory
 * segmentation, containing entries that describe memory segments including their
 * base address, size, and access privileges.
 * 
 * With this kernel, the only entries in the GDT are the null descriptor, the kernel code
 * segment, and the kernel data segment (both of which span the entire 32-bit address space). 
 */

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