// Functionality for printing characters from the screen using VGA text mode.
#include "print.h"
#include "types.h"
#include "core/portio/portio.h"
#include "core/vga/vga.h"

uint16_t _row = 0;
uint16_t _col = 0;

uint8_t _current_color = 0x0F;


void print_char(char c) {
    if (c == '\n') {
        print_cursor_next_line();
        return;
    }

    if (c == '\b') {
        if (_col == 0) {
            if (_row == 0) {
                _row = VGA_ROWS - 1;
            } else {
                _row--;
            }
            _col = VGA_COLS - 1;
        } else {
            _col--;
        }
        vga_char_set(' ', _current_color, _row, _col);
        return;
    }
    
    vga_char_set(c, _current_color, _row, _col);

    if (++_col >= VGA_COLS) {
        print_cursor_next_line();
    }
}

void print(char* str) {
    while (*str != 0x00) { // check for null terminator
        // Print the next character
        print_char(*(str++));
    }
}

// Print a specified number of characters.
void print_count(char* str, unsigned int count) {
    for (int i = 0; i < count; i++) {
        print_char(str[i]);
    }
}

void print_num(unsigned int number, unsigned int radix) {
    print_num_x(number, radix, FALSE, 0);
}

// eXtended print_num
void print_num_x(unsigned int number, unsigned int radix, bool prefixed, unsigned int min_width) {
    unsigned char zero_ascii = '0';
    char digits_str[33] = {0}; // digits of number, listed backward. lowest radix will be 2, which is 32 digits for a 32 bit int.
    unsigned int i = 31; // Second to last index of string, last is reserved for null terminator
    while (i >= 0 && number != 0) {
        unsigned char digit = (number % radix) + zero_ascii;
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

void print_screen_fill(char character, uint8_t color) {
    for (int row = 0; row < VGA_ROWS; row++) {
        for (int col = 0; col < VGA_COLS; col++) {
            vga_char_set(character, color, row, col);
        }
    }
}

uint8_t print_color_get() {
    return _current_color;
}

void print_color_set(uint8_t color) {
    _current_color = color;
}


void print_buffer_scroll() {
    // Need to clear out top row and shift other rows one by one
    for (int r = 0; r < VGA_ROWS; r++) {
        for (int c = 0; c < VGA_COLS; c++) {
            uint32_t cur_row = OFFSET(r, c);
            if (r == VGA_ROWS - 1) {
                *(uint8_t*)(VGA_MEMORY + cur_row) = ' ';
                *(uint8_t*)(VGA_MEMORY + cur_row + 1) = _current_color;
            } else {
                uint32_t next_row = OFFSET(r+1, c);
                *(uint16_t*)(VGA_MEMORY + cur_row) = *(uint16_t*)(VGA_MEMORY + next_row);
            }
        }
    }
}

void print_cursor_next_line() {
    for (int i = _col; i < VGA_COLS; i++) {
        vga_char_set(' ', _current_color, _row, i); // print spaces till end of row
    }
    _col = 0;
    if (++_row >= VGA_ROWS) {
        _row = VGA_ROWS - 1;
        print_buffer_scroll();
    }
}

uint16_t print_cursor_row_get() {
    return _row;
};
uint16_t print_cursor_col_get() {
    return _col;
};

void print_cursor_set(uint16_t row, uint16_t col) {
    _row = row;
    _col = col;
}

void print_cursor_refresh() {
    vga_cursor_update(_row, _col);
}