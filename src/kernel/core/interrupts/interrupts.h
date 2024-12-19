#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "types.h"

// Port addrs for PICs
#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

// Commands for PIC, send via outb(PIC#_COMMAND, PIC_XYZ)

// End of interrupt (EOI) control word
#define PIC_CMD_EOI 0x20

// Initialization Control Words (ICWs)
#define PIC_ICW_1 0b00010001
#define PIC1_ICW_2 0x20 // PIC1 handles IRQs 0-7, map to vectors 0x20-0x27
#define PIC2_ICW_2 0x28 // PIC2 handles IRQs 8-15, map to vectors 0x28-0x2F
#define PIC1_ICW_3 0b00000100 // Master - IRQ 2 used for slave (selected via bit 2)
#define PIC2_ICW_3 0x02 // Slave - attached to IRQ 2
#define PIC_ICW_4 0b00000001

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
} __attribute__((__packed__)) InterruptDescriptor;

typedef struct {
    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
} __attribute__((__packed__)) InterruptFrame;

typedef struct {
    uint32_t error_code;
    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
} __attribute__((__packed__)) InterruptFrameWithError;


// General interrupt-related functions
void int_start();
void int_disable();
void int_enable();

// Interrupt Descriptor Table (IDT) related functions
void int_idt_setup();
void int_idt_load();

// PIC functions
void    int_pic_send_eoi();
void    int_pic_init();
uint8_t int_pic_mask_get(uint8_t pic);
void    int_pic_mask_set(uint8_t pic, uint8_t mask);

// Functions related to ISRs

// Register an ISR
void int_isr_register(uint8_t index, uint32_t routine);

// Stub ISRs
void int_isr_stub(InterruptFrame* frame, uint8_t code);
void int_isr_err_stub(InterruptFrameWithError* frame, uint8_t code);

// ISRs to freeze the system when a GP, DF, or DZ is encountered
void int_isr_fault_gp(InterruptFrameWithError* frame); // General Protection Fault
void int_isr_fault_df(InterruptFrameWithError* frame); // Double Fault
void int_isr_fault_dbz(InterruptFrame* frame); // Divide by Zero

// Macros for registering ISRs
#define ADD_ISR(i, isr) int_isr_register(i, (uint32_t)isr)

// Note: ADD_ISR_STUB and ADD_ISR_ERR_STUB require using a corresponding
//       DEF_ISR_STUB or DEF_ISR_ERR_STUB.

// Register stub isr handler for normal interrupts
#define ADD_ISR_STUB(i) ADD_ISR(i, isr_stub_##i)
// Register stub isr handler for interrupts that also push an error code onto the stack
#define ADD_ISR_ERR_STUB(i) ADD_ISR(i, isr_err_stub_##i)


// Macros for defining unique stub ISRs for each code.
// e.g.
//      isr_stub_0x10(InterruptFrame* frame)
//      isr_err_stub_0x20(InterruptFrameWithError* frame)
// These invoke isr_debug or isr_err_debug
#define DEF_ISR_STUB(i) \
    __attribute__((interrupt))\
    void isr_stub_##i(InterruptFrame* frame) {\
        int_isr_stub(frame, i);\
    }

#define DEF_ISR_ERR_STUB(i) \
    __attribute__((interrupt))\
    void isr_err_stub_##i(InterruptFrameWithError* frame) {\
        int_isr_err_stub(frame, i);\
    }

#define DEF_ISR_STUB_IRQ(i) \
    __attribute__((interrupt))\
    void isr_stub_##i(InterruptFrame* frame) {\
        int_isr_stub(frame, i);\
        int_pic_send_eoi();\
    }

#endif