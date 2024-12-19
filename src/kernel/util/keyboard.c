#include "util/keyboard.h"
#include "types.h"
#include "util/logging.h"
#include "util/print.h"
#include "core/portio/portio.h"
#include "core/interrupts/interrupts.h"

void keyboard_init() {
    ADD_ISR(0x21, keyboard_isr); // Initialize keyboard IRQ
}

__attribute__((interrupt))
void keyboard_isr(InterruptFrame* frame) {
    // Go to dedicated keyboard handler function
    keyboard_scancode_recieve();

    int_pic_send_eoi(); // required for IRQs
}

bool lshift = FALSE;
bool rshift = FALSE;

void keyboard_scancode_recieve() {
    // Wait until kb is ready to be read by checking status
    uint8_t kb_status;
    while (!(kb_status & 0x01)) {
        kb_status = inb(KB_STATUS);
    }

    // Once kb ready, read in scancode
    uint8_t sc = inb(KB_OUTPUT);
    // Check if 
    switch(sc) {

    case (uint8_t)0x2a:
        lshift = TRUE;
    case (uint8_t)0x36: // lshift or rshift
        rshift = TRUE;
        return;
    case (uint8_t)0xaa:
        lshift = FALSE;
    case (uint8_t)0xb6:
        rshift = FALSE;
        return;
    }

    if (sc & 0x80) return; // If 8th bit set, this is a key release

    log_number_at("Scancode", sc, 16, 10, 30);

    char ch = keyboard_scancode_get_char(sc, lshift || rshift);

    if (ch) print_char(ch); 
}

char keyboard_scancode_get_char(uint8_t scancode, bool shift) {
    char scancode_without_release = scancode & 0x7F; // ignore 8th bit (release bit)
    return shift ? sc2ch_shift[scancode_without_release] : sc2ch[scancode_without_release];
}