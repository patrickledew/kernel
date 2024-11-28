#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "types.h"

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)
#define PIC_EOI 0x20

typedef struct{
    uint16_t size;
    uint32_t offset;
} __attribute__((__packed__)) IDTDescriptor;

typedef struct {
   uint16_t offset_1;        // offset bits 0..15
   uint16_t selector;        // a code segment selector in GDT or LDT
   uint8_t  __reserved;            // unused, set to 0
   uint8_t  type_attributes; // gate type, dpl, and p fields
   uint16_t offset_2;        // offset bits 16..31
}  InterruptDescriptor;

typedef struct {
    uint32_t error_code;
    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
} __attribute__((__packed__)) InterruptFrame;

void iret();

void int_disable();
void int_enable();

void init_idt();

// Send EOI (End of interrupt) signal to PIC
void pic_eoi();

#endif