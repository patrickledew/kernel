#ifndef VGA_H
#define VGA_H

#include "types.h"

#define OFFSET(row, col) ((row) * VGA_COLS + (col)) * 2

#define VGA_COLS 80
#define VGA_ROWS 25
#define VGA_MEMORY ((uint8_t*)0xB8000)


void vga_reg_set(uint16_t address_port, uint16_t data_port, uint8_t reg, uint8_t byte);
uint8_t vga_reg_get(uint16_t address_port, uint16_t data_port, uint8_t reg);

void vga_cursor_update(uint16_t row, uint16_t col);

void vga_char_set(char c, uint8_t color, uint16_t row, uint16_t col);

#endif