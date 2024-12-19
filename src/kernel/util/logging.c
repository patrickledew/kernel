#include "print.h"

void log_info(char* str) {
    println(str);
}

void log_number(char* str, unsigned int num, unsigned int radix) {
    print(str);
    print(": ");
    print_num_x(num, radix, TRUE, 4);
    print("\n");
}

void log_number_at(char* label, unsigned int num, unsigned int radix, uint16_t row, uint16_t col) {
    uint16_t prev_row = print_cursor_row_get();
    uint16_t prev_col = print_cursor_col_get();
    print_cursor_set(row, col);
    print(label);
    print(": ");
    print_num_x(num, radix, TRUE, 4);
    print_cursor_set(prev_row, prev_col);
}

void log_error(char* str) {
    uint8_t prev_color = print_color_get();
    print_color_set(0x0C);
    println(str);
    print_color_set(prev_color);
}

void log_memory(char* start_addr, uint32_t length) {
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