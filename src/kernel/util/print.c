// Functionality for printing characters from the screen using VGA text mode.
#include "print.h"
#include "types.h"
#include "portio.h"

short _row = 0;
short _col = 0;

uint8_t _current_color = 0x0F;


void print_char_at(char c, uint8_t color, short row, short col) {
    char* ptr = VIDEO_MEMORY + OFFSET(row, col);
    *(ptr++) = c;
    *(ptr) = color;
}

void print(char* str) {
    while (*str != 0x00) { // check for null pointer
        if (*str == '\n') {
            newline();
            str++;
            continue;
        }
        // Print the next character
        print_char_at(*(str++), _current_color, _row, _col);

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

void print_num(unsigned int number, unsigned int radix) {
    print_num_x(number, radix, FALSE, 0);
}

// eXtended print_num
void print_num_x(unsigned int number, unsigned int radix, bool prefixed, unsigned int min_width) {
    const char zero_ascii = '0';
    char digits_str[33] = {0}; // digits of number, listed backward. lowest radix will be 2, which is 32 digits for a 32 bit int.
    int i = 31; // Second to last index of string, last is reserved for null terminator
    while (i >= 0 && number != 0) {
        char digit = (number % radix) + zero_ascii;
        if (digit > '9') digit += 'A' - '9' - 1; // alpha chars don't come immediately after numerals, add offset
        digits_str[i--] = digit; // populate digits string backward, based on modulo
        number = number / radix;
    }
   
    while (i > 31 - min_width) {
        digits_str[i--] = zero_ascii;
    }
    if (prefixed) {
        if (radix == 16) {
            print("0x");
        } else if (radix == 2) {
            print("0b");
        }
    }
    print(digits_str + i + 1);
}

void println(char* str) {
    print(str);
    print("\n");
}

void fill_screen(char character, uint8_t color) {
    for (int row = 0; row < VIDEO_ROWS; row++) {
        for (int col = 0; col < VIDEO_COLS; col++) {
            print_char_at(character, color, row, col);
        }
    }
}

uint8_t get_color() {
    return _current_color;
}

void set_color(uint8_t color) {
    _current_color = color;
}


void scroll_buffer() {
    _row = 0;
}

void newline() {
    for (int i = _col; i < VIDEO_COLS; i++) {
        print_char_at(' ', _current_color, _row, i); // print spaces till end of row
    }
    _col = 0;
    _row++; // go to next row
    if (_row > VIDEO_ROWS) {
        scroll_buffer(); // not implemented yet
    }
}

/**
    Manipulate data inside VGA registers.
    address_port - VGA Address I/O port to use
    data_port - VGA Data I/O port to use
    reg - VGA register selector, e.g. 0x0F for cursor low, 0x0E for cursor high
    byte - Byte to load into the register
 */

void set_vga_reg(uint16_t address_port, uint16_t data_port, uint8_t reg, uint8_t byte) {
       outb(address_port, reg);
       outb(data_port, byte);
}

uint16_t get_cursor_row() {
    return _row;
};
uint16_t get_cursor_col() {
    return _col;
};


void set_cursor_pos(uint16_t row, uint16_t col) {
    _row = row;
    _col = col;
    uint16_t offset = OFFSET(row, col) / 2;
    char send_byte = (char)offset;
    set_vga_reg(0x03d4, 0x03d5, 0x0F, send_byte);
    send_byte >>= 8;
    set_vga_reg(0x03d4, 0x03d5, 0x0E, send_byte);
}