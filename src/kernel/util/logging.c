#include "print.h"

void log_info(char* str) {
    uint8_t prev_color = get_color();
    set_color(0x0F);
    println(str);
    set_color(prev_color);
}

void log_number(char* str, unsigned int num, unsigned int radix) {
    print(str);
    print(": ");
    print_num_x(num, radix, TRUE, 4);
    print("\n");
}

void log_number_at(char* label, unsigned int num, unsigned int radix, uint16_t row, uint16_t col) {
    uint16_t prev_row = get_cursor_row();
    uint16_t prev_col = get_cursor_col();
    set_cursor_pos(row, col);
    print(label);
    print(": ");
    print_num_x(num, radix, TRUE, 4);
    set_cursor_pos(prev_row, prev_col);
}

void log_error(char* str) {
    uint8_t prev_color = get_color();
    set_color(0x0C);
    println(str);
    set_color(prev_color);
}

void hex_dump(char* start_addr, uint32_t length) {
    const int word_size = 2; // in bytes
    const int words_per_row = 8;
    uint32_t mask = 0xFFFFFFFF >> (word_size * 8); // will right shift mask based on units displayed
    // default layout: 0x[start addr] 01 23 45 67 89 AB CD EF
    print("Dump of ");
    print_num_x((unsigned int)start_addr, 16, TRUE, 8);
    print(" - ");
    print_num_x((unsigned int)(start_addr + length), 16, TRUE, 8);
    println(":");
    char* cur_addr = start_addr;
    while (cur_addr < start_addr + length) {
        uint32_t offset = start_addr - cur_addr;
        if (offset % (word_size * words_per_row) == 0) {
            print("\n");
            print_num_x((uint32_t)cur_addr, 16, TRUE, 8);
            print(": ");
        }
        if (offset % word_size == 0) {
            uint32_t value = *((uint32_t*)cur_addr);
            value &= mask;
            print_num_x(value, 16, FALSE, word_size * 2);
            print(" ");

        }
        cur_addr++;
    }
    print("\n");
}