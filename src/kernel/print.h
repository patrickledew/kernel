#ifndef PRINT_H
#define PRINT_H

#include "types.h"

#define OFFSET(row, col) (row * VIDEO_COLS + col) * 2

#define VIDEO_COLS 80
#define VIDEO_ROWS 25
#define VIDEO_MEMORY ((char*)0xB8000)

void fill_screen(char c, char color);
void print_char(char c, char color, short row, short col);
void set_cursor_pos(uint16_t row, uint16_t col);

void print_num(unsigned int number, unsigned int radix, char color_code);
void print(char* str, char color_code);

#endif