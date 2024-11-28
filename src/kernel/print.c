// Functionality for printing characters from the screen using VGA text mode.
#include "print.h"

short _row = 0;
short _col = 0;

void fill_screen(char character, char color) {
    for (int row = 0; row < VIDEO_ROWS; row++) {
        for (int col = 0; col < VIDEO_COLS; col++) {
            print_char(character, color, row, col);
        }
    }
}

void print_char(char c, char color, short row, short col) {
    char* ptr = VIDEO_MEMORY + OFFSET(row, col);
    *(ptr++) = c;
    *(ptr) = color;
}

void scroll_buffer() {
    
}

void newline(char color) {
    for (int i = _col; i < VIDEO_COLS; i++) {
        print_char(' ', color, _row, i); // print spaces till end of row
    }
    _col = 0;
    _row++; // go to next row
    if (_row > VIDEO_ROWS) {
        scroll_buffer(); // not implemented yet
    }
}

void print_num(unsigned int number, unsigned int radix, char color_code) {
    const char zero_ascii = '0';
    char digits_str[33] = "00000000000000000000000000000000"; // digits of number, listed backward. lowest radix will be 2, which is 32 digits for a 32 bit int.
    int i = 31; // Second to last index of string, last is reserved for null terminator
    while (i >= 0 && number != 0) {
        char c = (number % radix) + zero_ascii;
        if (c > '9') c += 'A' - '9' - 1; // alpha chars don't come immediately after numerals, add offset
        digits_str[i] = c; // populate digits string backward, based on modulo
        number = number / radix;
        i--;
    }
    if (i > 31-4) {
        i = 31-4;
    }
    if (radix == 16) {
        print("0x", color_code);
    } else if (radix == 2) {
        print("0b", color_code);
    }
    // i will now be one less than the index of the first character in the string
    print(digits_str + i + 1, color_code);
}

void print(char* str, char color) {
    while (*str != 0x00) { // check for null pointer
        if (*str == '\n') {
            newline(color);
            str++;
            continue;
        }
        // Print the next character
        print_char(*str++, color, _row, _col);

        _col++;
        if (_col > VIDEO_COLS) {
            _col = 0;
            _row++;
            if (_row > VIDEO_ROWS) {
                scroll_buffer(); // not implemented yet
            }
        }
    }
    set_cursor_pos(_row, _col);
}

/**
    Manipulate data inside VGA registers.
    address_port - VGA Address I/O port to use
    data_port - VGA Data I/O port to use
    reg - VGA register selector, e.g. 0x0F for cursor low, 0x0E for cursor high
    byte - Byte to load into the register
 */

void set_vga_reg(uint16_t address_port, uint16_t data_port, uint8_t reg, uint8_t byte) {
       __asm__("out %%al, %%dx" : : "a"(reg), "d"(address_port));
       __asm__("out %%al, %%dx" : : "a"(byte), "d"(data_port));
}

void set_cursor_pos(uint16_t row, uint16_t col) {
    _row = row;
    _col = col;
    uint16_t offset = OFFSET(row, col) / 2;
    char send_byte = (char)offset;
    set_vga_reg(0x03d4, 0x03d5, 0x0F, send_byte);
    send_byte >>= 8;
    set_vga_reg(0x03d4, 0x03d5, 0x0E, send_byte);
}