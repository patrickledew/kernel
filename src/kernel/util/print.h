#ifndef PRINT_H
#define PRINT_H

#include "types.h"

#define OFFSET(row, col) ((row) * VIDEO_COLS + (col)) * 2

#define VIDEO_COLS 80
#define VIDEO_ROWS 25
#define VIDEO_MEMORY ((uint8_t*)0xB8000)

void fill_screen(char c, uint8_t color);

// Printing individual characters
void print_char_at(char c, uint8_t color, short row, short col);
void print_char(char c);

// Printing strings and data
void print(char* str);
void print_count(char* str, unsigned int count);
void println(char* str);
void print_num(unsigned int number, unsigned int radix);
void print_num_x(unsigned int number, unsigned int radix, bool prefixed, unsigned int min_width);

uint8_t get_color();
void set_color(uint8_t color);

uint16_t get_cursor_row();
uint16_t get_cursor_col();
void set_cursor_pos(uint16_t row, uint16_t col);
void newline();
void scroll_buffer();
void update_cursor();

#endif