#include "vga.h"
#include "core/portio/portio.h"
/*
    Manipulate data inside VGA registers.
    address_port - VGA Address I/O port to use
    data_port - VGA Data I/O port to use
    reg - VGA register selector, e.g. 0x0F for cursor low, 0x0E for cursor high
    byte - Byte to load into the register
 */

void vga_reg_set(uint16_t address_port, uint16_t data_port, uint8_t reg, uint8_t byte) {
       outb(address_port, reg);
       outb(data_port, byte);
}
uint8_t vga_reg_get(uint16_t address_port, uint16_t data_port, uint8_t reg) {
       outb(address_port, reg);
       return inb(data_port);
}

// Update cursor
void vga_cursor_update(uint16_t row, uint16_t col) {
    uint16_t offset = row * VGA_COLS + col;
    
    vga_reg_set(0x03d4, 0x03d5, 0x0F, offset & 0xFF);
    vga_reg_set(0x03d4, 0x03d5, 0x0E, (offset >> 8) & 0xFF);
}
void vga_char_set(char c, uint8_t color, uint16_t row, uint16_t col) {
    char* ptr = VGA_MEMORY + OFFSET(row, col);
    *(ptr++) = c;
    *(ptr) = color;
}