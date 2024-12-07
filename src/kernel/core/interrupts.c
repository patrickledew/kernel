#include "interrupts.h"
#include "util/print.h"
#include "util/portio.h"
#include "util/logging.h"
#include "util/keyboard.h"
#define NUM_IDT_DESCRIPTORS 1

// The IDT Descriptor (IDTR) tells the LIDT instruction where the IDT is located and how big it is.
idt_descriptor _idtr = {0};

// The Interrupt Descriptor Table (IDT).
// Holds 256 entries that correspond to different interrupt codes.
interrupt_descriptor _idt[256] = {0};

DEF_ISR_STUB(0x00);         // #DE Divide by 0
DEF_ISR_STUB(0x01);         // #DB Debug
DEF_ISR_STUB(0x02);         // Non-maskable interrupt
DEF_ISR_STUB(0x03);         // #BP Breakpoint
DEF_ISR_STUB(0x04);         // #OF Overflow
DEF_ISR_STUB(0x05);         // #BR Bound range exceeded
DEF_ISR_STUB(0x06);         // #UD Invalid opcode
DEF_ISR_STUB(0x07);         // #NM Device not available
DEF_ISR_ERR_STUB(0x08);     // #DF Double fault (also default hardware timer, but will remap)
DEF_ISR_STUB(0x09);         // Coprocessor segment overrun
DEF_ISR_ERR_STUB(0x0A);     // #TS Invalid TSS
DEF_ISR_ERR_STUB(0x0B);     // #NP Segment not present
DEF_ISR_ERR_STUB(0x0C);     // #SS Stack-segment fault
DEF_ISR_ERR_STUB(0x0D);     // #GP General protection fault
DEF_ISR_ERR_STUB(0x0E);     // #PF Page fault
DEF_ISR_STUB(0x0F);         // Reserved
DEF_ISR_STUB(0x10);         // #MF x87 FP exception
DEF_ISR_ERR_STUB(0x11);     // #AC Alignment check
DEF_ISR_STUB(0x12);         // #MC Machine check
DEF_ISR_STUB(0x13);         // #XM/#XF SIMD Floating-point exception
DEF_ISR_STUB(0x14);         // #VE Virtualization exception
DEF_ISR_STUB(0x15);         // #CP Control protection exception
DEF_ISR_STUB(0x16);         // Reserved
DEF_ISR_STUB(0x17);         // Reserved
DEF_ISR_STUB(0x18);         // Reserved
DEF_ISR_STUB(0x19);         // Reserved
DEF_ISR_STUB(0x1A);         // Reserved
DEF_ISR_STUB(0x1B);         // Reserved
DEF_ISR_STUB(0x1C);         // #HV Hypervisor injection exception
DEF_ISR_STUB(0x1D);         // #VC VMM communication exception
DEF_ISR_ERR_STUB(0x1E);     // #SX Security exception
DEF_ISR_STUB(0x1F);         // Reserved

// Hardware interrupts (IRQs) - 0x20-0x2F
DEF_ISR_STUB_IRQ(0x20); // Timer
DEF_ISR_STUB_IRQ(0x21); // Keyboard
DEF_ISR_STUB_IRQ(0x22);
DEF_ISR_STUB_IRQ(0x23);
DEF_ISR_STUB_IRQ(0x24);
DEF_ISR_STUB_IRQ(0x25);
DEF_ISR_STUB_IRQ(0x26);
DEF_ISR_STUB_IRQ(0x27);
DEF_ISR_STUB_IRQ(0x28);
DEF_ISR_STUB_IRQ(0x29);
DEF_ISR_STUB_IRQ(0x2A);
DEF_ISR_STUB_IRQ(0x2B);
DEF_ISR_STUB_IRQ(0x2C);
DEF_ISR_STUB_IRQ(0x2D);
DEF_ISR_STUB_IRQ(0x2E);
DEF_ISR_STUB_IRQ(0x2F);

// Sets up the Interrupt Descriptor Table (IDT)
void init_idt() {
    // Populate IDT descriptor
    _idtr.offset = (uint32_t)&_idt;
    _idtr.size = 0xFF * 0x08; // 256 entries

    // Next, setup interrupt routines and load IDT

    // Generate stubs for each of the lowest 32 exceptions
    ADD_ISR_STUB(0x00);         // #DE Divide by 0
    ADD_ISR_STUB(0x01);         // #DB Debug
    ADD_ISR_STUB(0x02);         // Non-maskable interrupt
    ADD_ISR_STUB(0x03);         // #BP Breakpoint
    ADD_ISR_STUB(0x04);         // #OF Overflow
    ADD_ISR_STUB(0x05);         // #BR Bound range exceeded
    ADD_ISR_STUB(0x06);         // #UD Invalid opcode
    ADD_ISR_STUB(0x07);         // #NM Device not available
    ADD_ISR_ERR_STUB(0x08);     // #DF Double fault (also default hardware timer, but that is remapped)
    ADD_ISR_STUB(0x09);         // Keyboard input
    ADD_ISR_ERR_STUB(0x0A);     // #TS Invalid TSS
    ADD_ISR_ERR_STUB(0x0B);     // #NP Segment not present
    ADD_ISR_ERR_STUB(0x0C);     // #SS Stack-segment fault
    ADD_ISR_ERR_STUB(0x0D);     // #GP General protection fault
    ADD_ISR_ERR_STUB(0x0E);     // #PF Page fault
    ADD_ISR_STUB(0x0F);         // Reserved
    ADD_ISR_STUB(0x10);         // #MF x87 FP exception
    ADD_ISR_ERR_STUB(0x11);     // #AC Alignment check
    ADD_ISR_STUB(0x12);         // #MC Machine check
    ADD_ISR_STUB(0x13);         // #XM/#XF SIMD Floating-point exception
    ADD_ISR_STUB(0x14);         // #VE Virtualization exception
    ADD_ISR_STUB(0x15);         // #CP Control protection exception
    ADD_ISR_STUB(0x16);         // Reserved
    ADD_ISR_STUB(0x17);         // Reserved
    ADD_ISR_STUB(0x18);         // Reserved
    ADD_ISR_STUB(0x19);         // Reserved
    ADD_ISR_STUB(0x1A);         // Reserved
    ADD_ISR_STUB(0x1B);         // Reserved
    ADD_ISR_STUB(0x1C);         // #HV Hypervisor injection exception
    ADD_ISR_STUB(0x1D);         // #VC VMM communication exception
    ADD_ISR_ERR_STUB(0x1E);     // #SX Security exception
    ADD_ISR_STUB(0x1F);         // Reserved

    // Hardware interrupts (IRQs)
    ADD_ISR_STUB(0x20); // Timer
    ADD_ISR_STUB(0x21);
    ADD_ISR_STUB(0x22);
    ADD_ISR_STUB(0x23);
    ADD_ISR_STUB(0x24);
    ADD_ISR_STUB(0x25);
    ADD_ISR_STUB(0x26);
    ADD_ISR_STUB(0x27);
    ADD_ISR_STUB(0x28);
    ADD_ISR_STUB(0x29);
    ADD_ISR_STUB(0x2A);
    ADD_ISR_STUB(0x2B);
    ADD_ISR_STUB(0x2C);
    ADD_ISR_STUB(0x2D);
    ADD_ISR_STUB(0x2E);
    ADD_ISR_STUB(0x2F);

    // Add custom ISRs
    ADD_ISR(0x00, divide_by_zero);
    ADD_ISR(0x0D, general_protection_fault);
    ADD_ISR(0x08, double_fault);
    ADD_ISR(0x20, timer);
    ADD_ISR(0x21, keyboard_isr);
  
    // Load the IDT
    load_idt();

    pic_init();
    // Finally, reenable interrupts
    int_enable();
}

void add_idt_desc(int index, uint32_t routine) {
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

void int_disable() {
    __asm__("cli");
}

void int_enable() {
    __asm__("sti");
}

// Initializes the PICs and remaps their interrupts to not conflict with
// x86 exception interrupts at 0x0-0x1F.
void pic_init() {
    // PIC initialization is done through a series of command words, ICW_1 through ICW_4.
    // ICW 1 - same for both
    // Bit 5 in this word is the initialization bit, which kicks off the init sequence
    outb(PIC1_COMMAND, PIC_ICW_1);
    outb(PIC2_COMMAND, PIC_ICW_1);
    // ICW 2 - this remaps the IRQs to start at 0x20 instead of 0x00
    // Since PIC1 has IRQs 0 - 7, and PIC2 has 8 - 15, map them separately
    // This is send to the PICs' data registers (ports 0x21 and 0xA1)
    outb(PIC1_DATA, PIC1_ICW_2);
    outb(PIC2_DATA, PIC2_ICW_2)
    // ICW 3 - this sets up cascading for the two PICs.
    // It designates an IRQ to use for communication between the two PICs.
    // For master, a bit mask indicates which IRQs have a slave PIC connected.
    // For slave, the lower 3 bits indicate which IRQ it is connected to
    //    (can also be thought as the slave ID)
    outb(PIC1_DATA, PIC1_ICW_3); 
    outb(PIC2_DATA, PIC2_ICW_3);
    // ICW 4
    outb(PIC1_DATA, PIC_ICW_4);
    outb(PIC2_DATA, PIC_ICW_4);

    // Now enable all IRQs (0 is active, 1 inactive)
    pic_set_mask(1, 0x00);
    pic_set_mask(2, 0x00);
}

uint8_t pic_get_mask(uint8_t pic) {
    uint8_t ret;
    switch (pic) {
        case 1: inb(PIC1_DATA, ret); break;
        case 2: inb(PIC2_DATA, ret); break;
    }
    return ret;
}

void pic_set_mask(uint8_t pic, uint8_t mask) {
    switch (pic) {
        case 1: outb(PIC1_DATA, mask); break;
        case 2: outb(PIC2_DATA, mask); break;
    }
}

// Sends an "End of interrupt" (EOI) command to the master and slave PICs (Programmable Interrupt Controllers)
// For IRQs, this will indicate to the PIC that the interrupt has been handled.
void pic_eoi() {
    outb(PIC1_COMMAND, PIC_CMD_EOI); // Master
    outb(PIC2_COMMAND, PIC_CMD_EOI); // Slave
}

/** The rest of this file contains interrupt handlers */

// isr_stub and isr_err_stub are called by ISRs defined
// with the DEF_ISR_STUB and DEF_ISR_ERR_STUB macros.
// 
// This allows us to see exactly which code is displayed for each interrupt triggered.
void isr_stub(interrupt_frame* frame, uint8_t code) {
    set_color(0x0D);
    log_number_at("Unhandled interrupt", code, 16, 0, 60);
    log_number_at("IP", frame->ip, 16, 2, 60);
    log_number_at("CS", frame->cs, 16, 3, 60);
    log_number_at("FLAGS", frame->flags, 16, 4, 60);
}

void isr_err_stub(interrupt_frame_err* frame, uint8_t code) {
    set_color(0x0D);
    log_number_at("Unhandled interrupt", code, 16, 0, 60);
    log_number_at("Error code", frame->error_code, 16, 1, 60);
    log_number_at("IP", frame->ip, 16, 2, 60);
    log_number_at("CS", frame->cs, 16, 3, 60);
    log_number_at("FLAGS", frame->flags, 16, 4, 60);
}

/* Handle #GP, usually caused by corrupt descriptor tables and some other critical errors */
// This halts the system and displays debug info.
__attribute__((interrupt))
void general_protection_fault(interrupt_frame_err* frame) {
    int_disable();
    set_color(0x4F);
    fill_screen(' ', 0x4F);
    set_cursor_pos(2, 0);
    print("#GP Occurred. Halting kernel.\n");
    set_cursor_pos (1, 0);

    log_number("Error Code", frame->error_code, 16);
    log_number("IP", frame->ip, 16);
    log_number("CS", frame->cs, 16);
    log_number("FLAGS", frame->flags, 16);

    if (frame->error_code != 0) {
        uint16_t ext = frame->error_code & 0b1;
        uint16_t table = (frame->error_code & 0b110) >> 1;
        uint16_t index = (frame->error_code & 0xFFF4) >> 3;

        println("#GP segmentation related.");
        print("External: ");
        println(ext ? "Yes" : "No");
        if (table == 0b00) {
            print("GDT ");
        } else if (table == 0b01 || table == 0b11) {
            print("IDT ");
        } else if (table == 0b10) {
            print("LDT ");
        }
        log_number(" Index", index, 16);
    }

    __asm__("hlt"); // Stop forever
}

/* Also handle double faults by freezing the system. */
__attribute__((interrupt))
void double_fault(interrupt_frame_err* frame) {
    int_disable();
    set_color(0x3F);
    fill_screen(' ', 0x3F);
    set_cursor_pos(0, 0);
    log_error("#DF Occurred. Halting kernel.\n");
    log_number("Error Code (Should be 0)", frame->error_code, 16);
    log_number("IP", frame->ip, 16);
    log_number("CS", frame->cs, 16);
    log_number("FLAGS", frame->flags, 16);

    __asm__("hlt"); // Stop forever
}

__attribute__((interrupt))
void divide_by_zero(interrupt_frame* frame) {
    int_disable();
    set_color(0x0C);
    fill_screen(' ', 0x0C);
    set_cursor_pos(0, 0);
    log_error("Divide by zero. Halting Kernel.\n");
    log_number("IP", frame->ip, 16);
    log_number("CS", frame->cs, 16);
    log_number("FLAGS", frame->flags, 16);
    __asm__("hlt");
}


volatile int tick = 0;
__attribute__((interrupt))
void timer(interrupt_frame* frame) {
    set_color(0x0F);
    log_number_at("Tick", tick++, 16, VIDEO_ROWS - 1, VIDEO_COLS - 20);
    update_cursor();
    pic_eoi();
}

__attribute__((interrupt))
void keyboard_isr(interrupt_frame* frame) {
    // Go to dedicated keyboard handler function
    keyboard_handler();

    pic_eoi(); // required for IRQs
}