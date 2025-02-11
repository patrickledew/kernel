#ifndef ASSERT_H
#define ASSERT_H

#include "types.h"

#define BREAK_ON_ASSERT_FAIL

void _assert_i(int expected, int actual, int line, char* file);
void _assert_b(bool expected, bool actual, int line, char* file);
void _assert_u32(uint32_t expected, uint32_t actual, int line, char* file);
void _assert_u16(uint16_t expected, uint16_t actual, int line, char* file);
void _assert_u8(uint8_t expected, uint8_t actual, int line, char* file);
void _assert_s(char* expected, char* actual, int line, char* file);

void _assert_not_i(int expected, int actual, int line, char* file);
void _assert_not_b(bool expected, bool actual, int line, char* file);
void _assert_not_u32(uint32_t expected, uint32_t actual, int line, char* file);
void _assert_not_u16(uint16_t expected, uint16_t actual, int line, char* file);
void _assert_not_u8(uint8_t expected, uint8_t actual, int line, char* file);
void _assert_not_s(char* expected, char* actual, int line, char* file);


void _assert_gt_i(int expected, int actual, int line, char* file);
void _assert_gt_u32(uint32_t expected, uint32_t actual, int line, char* file);
void _assert_gt_u16(uint16_t expected, uint16_t actual, int line, char* file);
void _assert_gt_u8(uint8_t expected, uint8_t actual, int line, char* file);

void _assert_lt_i(int expected, int actual, int line, char* file);
void _assert_lt_u32(uint32_t expected, uint32_t actual, int line, char* file);
void _assert_lt_u16(uint16_t expected, uint16_t actual, int line, char* file);
void _assert_lt_u8(uint8_t expected, uint8_t actual, int line, char* file);

void _assert_not_in_range_i(int expected_start, int expected_end, int actual, int line, char* file);
void _assert_not_in_range_u32(uint32_t expected_start, uint32_t expected_end, uint32_t actual, int line, char* file);
void _assert_not_in_range_u16(uint16_t expected_start, uint16_t expected_end, uint16_t actual, int line, char* file);
void _assert_not_in_range_u8(uint8_t expected_start, uint8_t expected_end, uint8_t actual, int line, char* file);


void assertion_failed(int line, char* file);

#define assert_b(expected, actual) _assert_b(expected, actual, __LINE__, __FILE__)
#define assert_i(expected, actual) _assert_i(expected, actual, __LINE__, __FILE__)
#define assert_u32(expected, actual) _assert_u32(expected, actual, __LINE__, __FILE__)
#define assert_u16(expected, actual) _assert_u16(expected, actual, __LINE__, __FILE__)
#define assert_u8(expected, actual) _assert_u8(expected, actual, __LINE__, __FILE__)
#define assert_s(expected, actual) _assert_s(expected, actual, __LINE__, __FILE__)

#define assert_not_b(expected, actual) _assert_not_b(expected, actual, __LINE__, __FILE__)
#define assert_not_i(expected, actual) _assert_not_i(expected, actual, __LINE__, __FILE__)
#define assert_not_u32(expected, actual) _assert_not_u32(expected, actual, __LINE__, __FILE__)
#define assert_not_u16(expected, actual) _assert_not_u16(expected, actual, __LINE__, __FILE__)
#define assert_not_u8(expected, actual) _assert_not_u8(expected, actual, __LINE__, __FILE__)
#define assert_not_s(expected, actual) _assert_not_s(expected, actual, __LINE__, __FILE__)

#define assert_gt_i(expected, actual) _assert_gt_i(expected, actual, __LINE__, __FILE__)
#define assert_gt_u32(expected, actual) _assert_gt_u32(expected, actual, __LINE__, __FILE__)
#define assert_gt_u16(expected, actual) _assert_gt_u16(expected, actual, __LINE__, __FILE__)
#define assert_gt_u8(expected, actual) _assert_gt_u8(expected, actual, __LINE__, __FILE__)

#define assert_lt_i(expected, actual) _assert_lt_i(expected, actual, __LINE__, __FILE__)
#define assert_lt_u32(expected, actual) _assert_lt_u32(expected, actual, __LINE__, __FILE__)
#define assert_lt_u16(expected, actual) _assert_lt_u16(expected, actual, __LINE__, __FILE__)
#define assert_lt_u8(expected, actual) _assert_lt_u8(expected, actual, __LINE__, __FILE__)

#define assert_not_in_range_i(expected_start, expected_end, actual) _assert_not_in_range_i(expected_start, expected_end, actual, __LINE__, __FILE__)
#define assert_not_in_range_u32(expected_start, expected_end, actual) _assert_not_in_range_u32(expected_start, expected_end, actual, __LINE__, __FILE__)
#define assert_not_in_range_u16(expected_start, expected_end, actual) _assert_not_in_range_u16(expected_start, expected_end, actual, __LINE__, __FILE__)
#define assert_not_in_range_u8(expected_start, expected_end, actual) _assert_not_in_range_u8(expected_start, expected_end, actual, __LINE__, __FILE__)


#endif