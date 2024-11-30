#ifndef KEYBOARD_H
#define KEYBOARD_H


// Keyboard I/O ports
#define KB_COMMAND 0x64
#define KB_DATA 0x60
#define KB_OUTPUT 0x60
#define KB_STATUS 0x64

// A list of keyboard scancodes
// Special Codes
#define SC_ERROR_1  0x00
#define SC_ERROR_2  0xFF
#define SC_INT_FAIL 0xFD
#define SC_RESEND   0xFE

void keyboard_init();
void keyboard_handler();

#endif