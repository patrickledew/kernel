#include "util/keyboard.h"
#include "types.h"
#include "util/logging.h"
#include "util/print.h"
#include "util/portio.h"

char* SC_TO_STR[256] = { 0 };

void keyboard_init() {
    SC_TO_STR[0x00] = "";
    SC_TO_STR[0x01] = "Esc";
    SC_TO_STR[0x02] = "1";
    SC_TO_STR[0x03] = "2";
    SC_TO_STR[0x04] = "3";
    SC_TO_STR[0x05] = "4";
    SC_TO_STR[0x06] = "5";
    SC_TO_STR[0x07] = "6";
    SC_TO_STR[0x08] = "7";
    SC_TO_STR[0x09] = "8";
    SC_TO_STR[0x0A] = "9";
    SC_TO_STR[0x0B] = "0";
    SC_TO_STR[0x0C] = "-",      
    SC_TO_STR[0x0D] = "=",      
    SC_TO_STR[0x0E] = "BkSp";
    SC_TO_STR[0x0F] = "Tab";
    SC_TO_STR[0x10] = "q";
    SC_TO_STR[0x11] = "w";
    SC_TO_STR[0x12] = "e";
    SC_TO_STR[0x13] = "r";
    SC_TO_STR[0x14] = "t";
    SC_TO_STR[0x15] = "y";
    SC_TO_STR[0x16] = "u";
    SC_TO_STR[0x17] = "i";
    SC_TO_STR[0x18] = "o";
    SC_TO_STR[0x19] = "p";
    SC_TO_STR[0x1A] = "[";      
    SC_TO_STR[0x1B] = "]";      
    SC_TO_STR[0x1C] = "Enter";
    SC_TO_STR[0x1D] = "LCtrl";
    SC_TO_STR[0x1E] = "a";
    SC_TO_STR[0x1F] = "s";
    SC_TO_STR[0x20] = "d";
    SC_TO_STR[0x21] = "f";
    SC_TO_STR[0x22] = "g";
    SC_TO_STR[0x23] = "h";
    SC_TO_STR[0x24] = "j";
    SC_TO_STR[0x25] = "k";
    SC_TO_STR[0x26] = "l";
    SC_TO_STR[0x27] = ";";      
    SC_TO_STR[0x28] = "'";      
    SC_TO_STR[0x29] = "`";      
    SC_TO_STR[0x2A] = "LShift";
    SC_TO_STR[0x2B] = "\\";     
    SC_TO_STR[0x2C] = "z";
    SC_TO_STR[0x2D] = "x";
    SC_TO_STR[0x2E] = "c";
    SC_TO_STR[0x2F] = "v";
    SC_TO_STR[0x30] = "b";
    SC_TO_STR[0x31] = "n";
    SC_TO_STR[0x32] = "m";
    SC_TO_STR[0x33] = ",";      
    SC_TO_STR[0x34] = ".";      
    SC_TO_STR[0x35] = "/";      
    SC_TO_STR[0x36] = "RShift";
    SC_TO_STR[0x37] = "KP-";
    SC_TO_STR[0x38] = "LAlt";
    SC_TO_STR[0x39] = " ";
    SC_TO_STR[0x3A] = "Caps";
    for (int i = 0x3B; i < 0xFF; i++) {
        SC_TO_STR[i] = "";
    }
}

void keyboard_handler() {
    set_color(0x0C);
    uint8_t kb_status;
    // Wait until kb is ready to be read
    while (!(kb_status & 0x01)) {
        inb(KB_STATUS, kb_status);
    }
    uint8_t kb_output;
    inb(KB_OUTPUT, kb_output);
    // Scancode will be in kb_output
    uint32_t idx = ((uint32_t)kb_output) & 0xFF;
    char* text = SC_TO_STR[idx];
    print(text);
}
