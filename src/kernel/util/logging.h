#ifndef LOGGING_H
#define LOGGING_H

#include "print.h"
#include "types.h"

// Prints w/ color 0x0F (white on black)
void log_info(char* str);
// Prints "[label]: [num]" w/ current color
void log_number(char* label, int num, unsigned int radix);
void log_number_u(char* label, unsigned int num, unsigned int radix);
void log_number_at(char* label, int num, unsigned int radix, uint16_t row, uint16_t col);
void log_number_at_u(char* label, unsigned int num, unsigned int radix, uint16_t row, uint16_t col);
// Prints w/ color 0x0c (red on black)
void log_error(char* str);

// Prints a dump of a memory region starting at start_addr.
void log_memory(char* start_addr, uint32_t length);

#endif