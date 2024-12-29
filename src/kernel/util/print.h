#ifndef PRINT_H
#define PRINT_H

#include "types.h"

void print_screen_fill(char c, uint8_t color);

// Printing individual characters
void print_char(char c);

// Printing strings and data
void print(char* str); // Printing null terminated string
void print_count(char* str, unsigned int count); // Printing a fixed-length string
void println(char* str);
void print_num(int number, unsigned int radix);
void print_num_u(unsigned int number, unsigned int radix);
void print_num_x(int number, unsigned int radix, bool prefixed, int min_width, bool is_signed);

uint8_t print_color_get();
void print_color_set(uint8_t color);

uint16_t print_cursor_row_get();
uint16_t print_cursor_col_get();
void print_cursor_set(uint16_t row, uint16_t col);
void print_cursor_next_line();
void print_cursor_refresh();

void print_buffer_scroll();

#endif