#include "util/keyboard.h"
#include "types.h"
#include "util/logging.h"
#include "util/print.h"
#include "util/portio.h"


void keyboard_init() {
   // nothing to do yet
}

bool lshift = FALSE;
bool rshift = FALSE;

void keyboard_handler() {
    // Wait until kb is ready to be read by checking status
    uint8_t kb_status;
    while (!(kb_status & 0x01)) {
        inb(KB_STATUS, kb_status);
    }

    // Once kb ready, read in scancode
    uint8_t sc;
    inb(KB_OUTPUT, sc);
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

    char ch = get_scancode_char(sc, lshift || rshift);

    if (ch) print_char(ch);
}

char get_scancode_char(uint8_t scancode, bool shift) {
    char scancode_without_release = scancode & 0x7F; // ignore 8th bit (release bit)
    return shift ? sc2ch_shift[scancode_without_release] : sc2ch[scancode_without_release];
}