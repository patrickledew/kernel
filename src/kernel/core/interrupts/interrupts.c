#include "interrupts.h"
#include "util/print.h"
#include "core/portio/portio.h"
#include "util/logging.h"
#include "util/keyboard.h"
#define NUM_IDT_DESCRIPTORS 1

// The IDT Descriptor (IDTR) tells the LIDT instruction where the IDT is located and how big it is.
IDTDescriptor _idtr = {0};

// The Interrupt Descriptor Table (IDT).
// Holds 256 entries that correspond to different interrupt codes.
InterruptDescriptor _idt[256] = {0};

__attribute__((interrupt))
void effnine(InterruptFrame* frame) {
    print_color_set(0x0F);
    print("Interrupt 0xF9 occurred. Halting kernel.\n");
    int_disable();
    __asm__("hlt");
}
// Sets up the Interrupt Descriptor Table (IDT)
void int_init() {
    log_info("int_init: initializing interrupt descriptor table.");
    // Populate IDT descriptor
    _idtr.offset = (uint32_t)&_idt;
    _idtr.size = 0xFF * 0x08; // 256 entries

    // Next, setup interrupt routines and load IDT

    // Register stub ISRs
    int_stub_register_all();
    log_info("int_init: registered all stub ISRs.");
    // Add error ISRs
    REG_ISR(0x00, int_isr_fault_dbz);
    REG_ISR(0x0D, int_isr_fault_gp);
    REG_ISR(0x08, int_isr_fault_df);
    REG_ISR(0x0E, int_isr_fault_pf);     // #PF Page fault

    // Configure PIC
    int_pic_init();

    // Don't enable interrupts yet as this can be done after other files register their interrupts
}

void int_start() {
    // Load the IDT
    int_idt_load();

    // Finally, reenable interrupts
    int_enable();
}

void int_isr_register(int index, void* routine, bool trap) {
    _idt[index].offset_1 = (uint16_t)((uint32_t)routine & 0xFFFF);
    _idt[index].offset_2 = (uint16_t)((uint32_t)routine >> 16);
    _idt[index].selector = 0x08 // Segment selector 0x08
                         | 0b0 << 2 // TI = GDT (0)
                         | 0b0; // RPL = 0
    _idt[index].type_attributes = 0b1110 // Gate type = 0x1110 = Interrupt Gate. 0x1111 would be Trap gate
                                | 0b00 << 5 // DPL - ring 0
                                | 0b1 << 7; // Present bit (always 1)
    if (trap) _idt[index].type_attributes |= 0b1; // Trap gate
}

void int_idt_load() {
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
void int_pic_init() {
    // PIC initialization is done through a series of command words, ICW_1 through ICW_4.
    // ICW 1 - same for both
    // Bit 5 in this word is the initialization bit, which kicks off the init sequence
    outb(PIC1_COMMAND, PIC_ICW_1);
    outb(PIC2_COMMAND, PIC_ICW_1);
    // ICW 2 - this remaps the IRQs to start at 0x20 instead of 0x00
    // Since PIC1 has IRQs 0 - 7, and PIC2 has 8 - 15, map them separately
    // This is send to the PICs' data registers (ports 0x21 and 0xA1)
    outb(PIC1_DATA, PIC1_ICW_2);
    outb(PIC2_DATA, PIC2_ICW_2);
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
    int_pic_mask_set(1, 0x00);
    int_pic_mask_set(2, 0x00);
}

uint8_t int_pic_mask_get(uint8_t pic) {
    switch (pic) {
        case 1: return inb(PIC1_DATA);
        case 2: return inb(PIC2_DATA);
    }
    return 0xFF;
}

void int_pic_mask_set(uint8_t pic, uint8_t mask) {
    switch (pic) {
        case 1: outb(PIC1_DATA, mask); break;
        case 2: outb(PIC2_DATA, mask); break;
    }
}

// Sends an "End of interrupt" (EOI) command to the master and slave PICs (Programmable Interrupt Controllers)
// For IRQs, this will indicate to the PIC that the interrupt has been handled.
void int_pic_send_eoi() {
    outb(PIC1_COMMAND, PIC_CMD_EOI); // Master
    outb(PIC2_COMMAND, PIC_CMD_EOI); // Slave
}

/** The rest of this file contains exception interrupt handlers and stubs */

// int_isr_stub and int_isr_err_stub are called by ISRs defined
// with the DEF_ISR_STUB and DEF_ISR_ERR_STUB macros.
// 
// This allows us to see exactly what the interrupt code is for each unhandled interrupt.
void int_isr_stub(InterruptFrame* frame, uint8_t irq) {
    int_disable();
    uint8_t c = print_color_get();
    print_color_set(0x0D);
    log_number_u("Unhandled interrupt", irq & 0xFF, 16);
    log_number_u("IP", frame->ip, 16);
    log_number_u("CS", frame->cs, 16);
    log_number_u("FLAGS", frame->flags, 16);
    print_color_set(c);
    __asm__("hlt");
}

void int_isr_err_stub(InterruptFrame* frame, uint32_t error_code, uint8_t irq) {
    uint8_t c = print_color_get();
    print_color_set(0x0D);
    log_number_u("Unhandled interrupt", irq, 16);
    log_number_u("Error code", error_code, 16);
    log_number_u("IP", frame->ip, 16);
    log_number_u("CS", frame->cs, 16);
    log_number_u("FLAGS", frame->flags, 16);
    print_color_set(c);
}

void int_isr_fault_common(InterruptFrame* frame, char* label) {
    print_color_set(0x4F);
    print_screen_fill(' ', 0x4F);
    print_cursor_set(0, 0);
    print(label);
    print(" Occurred. Halting kernel.\n");
    print_cursor_set (2, 0);

    log_number_u("IP", frame->ip, 16);
    log_number_u("CS", frame->cs, 16);
    log_number_u("FLAGS", frame->flags, 16);
}

void int_isr_fault_common_err(InterruptFrame* frame, uint32_t error_code, char* label) {
    print_color_set(0x4F);
    print_screen_fill(' ', 0x4F);
    print_cursor_set(0, 0);
    print(label);
    print(" Occurred. Halting kernel.\n");
    print_cursor_set (2, 0);

    log_number_u("Error Code", error_code, 16);
    log_number_u("IP", frame->ip, 16);
    log_number_u("CS", frame->cs, 16);
    log_number_u("FLAGS", frame->flags, 16);
}

/* Handle #GP, usually caused by corrupt descriptor tables and some other critical errors
 * Halts the system and displays debug info.
 */
__attribute__((interrupt))
void int_isr_fault_gp(InterruptFrame* frame, uint32_t error_code) {
    int_disable();

    int_isr_fault_common_err(frame, error_code, "#GP");

    if (error_code != 0) {
        uint16_t ext = error_code & 0b1;
        uint16_t table = (error_code & 0b110) >> 1;
        uint16_t index = (error_code & 0xFFF4) >> 3;

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
        log_number_u(" Index", index, 16);
    }

    __asm__("hlt"); // Stop forever
}

/* Also handle double faults by freezing the system. */
__attribute__((interrupt))
void int_isr_fault_df(InterruptFrame* frame, uint32_t error_code) {
    int_disable();
    int_isr_fault_common_err(frame, error_code, "#DF");
    __asm__("hlt"); // Stop forever
}

/* Same for dividing by zero. */
__attribute__((interrupt))
void int_isr_fault_dbz(InterruptFrame* frame) {
    int_disable();
    int_isr_fault_common(frame, "#DBZ");
    __asm__("hlt");
}

__attribute__((interrupt))
void int_isr_fault_pf(InterruptFrame* frame, uint32_t error_code) {
    int_disable();
    log_error("Page fault occurred. Halting kernel.");
    // int_isr_fault_common_err(frame, error_code, "#PF");

    uint32_t *cr2, *cr3;
    __asm__("mov %%cr2, %0; mov %%cr3, %1" : "=r"(cr2), "=r"(cr3));
    log_number_u("CR2 (Accessed Addr)", (uint32_t)cr2, 16);
    log_number_u("CR3 (Page Directory Addr)", (uint32_t)cr3, 16);

    uint32_t page_dir_index = (uint32_t)cr2 / 0x1000 / 1024;
    uint32_t page_table_index = (uint32_t)cr2 / 0x1000 % 1024;
    log_info("Page Directory Entries at and after accessed address:");
    // log_memory(&cr3[page_dir_index], 16);
    // uint32_t* page_table = (uint32_t*)cr3[page_dir_index];
    // log_info("Page Table Entries at and after accessed address:");
    // log_memory(&page_table[page_table_index], 16);
    __asm__("hlt");
}