#include "interrupts.h"
#include "print.h"
#include "io.h"
#define NUM_IDT_DESCRIPTORS 1

IDTDescriptor _idtr = {0};

InterruptDescriptor _idt[256] = {0};

inline void int_disable() {
    __asm__("cli");
}

inline void int_enable() {
    __asm__("sti");
}

__attribute__((interrupt))
void interrupt_stub(InterruptFrame* frame) {
    __asm__("nop");
}

__attribute__((interrupt))
void general_protection_fault(InterruptFrame* frame) {
    int_disable();
    fill_screen(' ', 0x4F);
    set_cursor_pos(1, 0);
    print("#GP Occurred. Halting kernel.\n", 0x4F);
    set_cursor_pos (5, 0);
    print("Error code: ", 0x4F);
    print_num(frame->error_code, 16, 0x4F);
    print("\nIP: ", 0x4F);
    print_num(frame->ip, 16, 0x4F);
    print("\nCS: ", 0x4F);
    print_num(frame->cs, 16, 0x4F);
    print("\nFLAGS: ", 0x4F);
    print_num(frame->flags, 2, 0x4F);
    // __asm__("hlt"); // Stop forever
}


__attribute__((interrupt))
void test_interrupt_handler(InterruptFrame* frame) {
    print("Interrupt reached!\n", 0xF0);
}

volatile int tick = 0;
__attribute__((interrupt))
void timer(InterruptFrame* frame) {
    set_cursor_pos(10, 0);
    print("\nTimer triggered.\n", 0x0F);
    print("Tick: ", 0x0F);
    print_num(tick++, 16, 0x0F);
    pic_eoi();
}

void add_idt_desc(int index, uint32_t routine) {
    _idtr.size += 0x08;
    _idt[index].offset_1 = (uint16_t)routine;
    _idt[index].offset_2 = (uint16_t)(routine >> 16);
    _idt[index].selector = 0x08 // Segment selector 0x08
                         | 0b0 << 2 // TI = GDT (0)
                         | 0b0; // RPL = 0
    _idt[index].type_attributes = 0b1110 // Gate type = 0x1110 = Interrupt Gate. 0x1111 would be Trap gate
                                | 0b00 << 5 // DPL - ring 0
                                | 0b1 << 7; // Present bit (always 1)

}

void load_idt() {
    __asm__("lidt %0" : : "m"(_idtr));
}
// Sets up the Interrupt Descriptor Table (IDT)
void init_idt() {
    // Populate IDT descriptor
    _idtr.offset = (uint32_t)&_idt;
    _idtr.size = 0;

    // Next, setup interrupt routines and load IDT
    for (int i = 0; i < 0xFF; i++) {
        add_idt_desc(i, (uint32_t)interrupt_stub);
    }
    add_idt_desc(0x0D, (uint32_t)general_protection_fault);
    add_idt_desc(0x80, (uint32_t)test_interrupt_handler);
    add_idt_desc(0x08, (uint32_t)timer);
  
    load_idt();

    // Finally, reenable interrupts
    int_enable();
}

void pic_eoi() {
    outb(PIC2_COMMAND, PIC_EOI); // Slave
    outb(PIC1_COMMAND, PIC_EOI); // Master
}