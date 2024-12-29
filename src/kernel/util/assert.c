#include "logging.h"
#include "strutil.h"

void debug_break() {
    // __asm__("int3"); // Not sure why this doesn't cause debugger to break
    __asm__("cli; hlt");
}

#define BREAK_ON_ASSERT_FAIL 1

void _assert_i(int expected, int actual, int line, char* file) {
    if (expected != actual) {
        uint8_t color = print_color_get();
        print_color_set(0x0C);
        print("Assertion failed: ");
        print(file); print(":"); print_num_u(line, 10);
        print(" (Expected: "); print_num(expected, 10);
        print(", Actual: "); print_num(actual, 10); println(")");
        print_color_set(color);
        #ifdef BREAK_ON_ASSERT_FAIL
        debug_break();
        #endif
    }
}

void _assert_b(bool expected, bool actual, int line, char* file) {
    if (expected != actual) {
        uint8_t color = print_color_get();
        print_color_set(0x0C);
        print("Assertion failed: ");
        print(file); print(":"); print_num_u(line, 10);
        print(" (Expected: ");
        if (expected == FALSE) print("FALSE"); else print("TRUE");
        print(", Actual: "); 
        if (actual == FALSE) print("FALSE"); else print("TRUE");
        println(")");
        print_color_set(color);
        #ifdef BREAK_ON_ASSERT_FAIL
        debug_break();
        #endif
    }
}

void _assert_u32(uint32_t expected, uint32_t actual, int line, char* file) {
    if (expected != actual) {
        uint8_t color = print_color_get();
        print_color_set(0x0C);
        print("Assertion failed: ");
        print(file); print(":"); print_num_u(line, 10);
        print("(Expected: "); print_num_u(expected, 10);
        print(", Actual: "); print_num_u(actual, 10); println(")");
        print_color_set(color);
        #ifdef BREAK_ON_ASSERT_FAIL
        debug_break();
        #endif
    }
}

void _assert_u16(uint16_t expected, uint16_t actual, int line, char* file) {
    if (expected != actual) {
        uint8_t color = print_color_get();
        print_color_set(0x0C);
        print("Assertion failed: ");
        print(file); print(":"); print_num_u(line, 10);
        print("(Expected: "); print_num_u(expected, 10);
        print(", Actual: "); print_num_u(actual, 10); println(")");
        print_color_set(color);
        #ifdef BREAK_ON_ASSERT_FAIL
        debug_break();
        #endif
    }
}

void _assert_u8(uint8_t expected, uint8_t actual, int line, char* file) {
    if (expected != actual) {
        uint8_t color = print_color_get();
        print_color_set(0x0C);
        print("Assertion failed: ");
        print(file); print(":"); print_num_u(line, 10);
        print("(Expected: "); print_num_u(expected, 10);
        print(", Actual: "); print_num_u(actual, 10); println(")");
        print_color_set(color);
        #ifdef BREAK_ON_ASSERT_FAIL
        debug_break();
        #endif
    }
}

void _assert_s(char* expected, char* actual, int line, char* file) {
    if (strcmp(expected, actual, 0) != 0) {
        uint8_t color = print_color_get();
        print_color_set(0x0C);
        print("Assertion failed: ");
        print(file); print(":"); print_num_u(line, 10);
        print(" (Expected: \""); print(expected);
        print("\", Actual: \""); print(actual); println("\")");
        print_color_set(color);
        #ifdef BREAK_ON_ASSERT_FAIL
        debug_break();
        #endif
    }
}

void _assert_not_i(int expected, int actual, int line, char* file) {
    if (expected == actual) {
        uint8_t color = print_color_get();
        print_color_set(0x0C);
        print("Assertion failed: ");
        print(file); print(":"); print_num_u(line, 10);
        print(" (Expected: Not "); print_num(expected, 10);
        print(", Actual: "); print_num(actual, 10); println(")");
        print_color_set(color);
        #ifdef BREAK_ON_ASSERT_FAIL
        debug_break();
        #endif
    }
}

void _assert_not_b(bool expected, bool actual, int line, char* file) {
    if (expected == actual) {
        uint8_t color = print_color_get();
        print_color_set(0x0C);
        print("Assertion failed: ");
        print(file); print(":"); print_num_u(line, 10);
        print(" (Expected: Not ");
        if (expected == FALSE) print("FALSE"); else print("TRUE");
        print(", Actual: "); 
        if (actual == FALSE) print("FALSE"); else print("TRUE");
        println(")");
        print_color_set(color);
        #ifdef BREAK_ON_ASSERT_FAIL
        debug_break();
        #endif
    }
}

void _assert_not_u32(uint32_t expected, uint32_t actual, int line, char* file) {
    if (expected == actual) {
        uint8_t color = print_color_get();
        print_color_set(0x0C);
        print("Assertion failed: ");
        print(file); print(":"); print_num_u(line, 10);
        print("(Expected: Not "); print_num_u(expected, 10);
        print(", Actual: "); print_num_u(actual, 10); println(")");
        print_color_set(color);
        #ifdef BREAK_ON_ASSERT_FAIL
        debug_break();
        #endif
    }
}

void _assert_not_u16(uint16_t expected, uint16_t actual, int line, char* file) {
    if (expected == actual) {
        uint8_t color = print_color_get();
        print_color_set(0x0C);
        print("Assertion failed: ");
        print(file); print(":"); print_num_u(line, 10);
        print("(Expected: Not "); print_num_u(expected, 10);
        print(", Actual: "); print_num_u(actual, 10); println(")");
        print_color_set(color);
        #ifdef BREAK_ON_ASSERT_FAIL
        debug_break();
        #endif
    }
}

void _assert_not_u8(uint8_t expected, uint8_t actual, int line, char* file) {
    if (expected == actual) {
        uint8_t color = print_color_get();
        print_color_set(0x0C);
        print("Assertion failed: ");
        print(file); print(":"); print_num_u(line, 10);
        print("(Expected: Not "); print_num_u(expected, 10);
        print(", Actual: "); print_num_u(actual, 10); println(")");
        print_color_set(color);
        #ifdef BREAK_ON_ASSERT_FAIL
        debug_break();
        #endif
    }
}

void _assert_not_s(char* expected, char* actual, int line, char* file) {
    if (strcmp(expected, actual, 0) == 0) {
        uint8_t color = print_color_get();
        print_color_set(0x0C);
        print("Assertion failed: ");
        print(file); print(":"); print_num_u(line, 10);
        print(" (Expected: Not \""); print(expected);
        print("\", Actual: \""); print(actual); println("\")");
        print_color_set(color);
        #ifdef BREAK_ON_ASSERT_FAIL
        debug_break();
        #endif
    }
}



void _assert_gt_i(int expected, int actual, int line, char* file) {
    if (!(actual > expected)) {
        uint8_t color = print_color_get();
        print_color_set(0x0C);
        print("Assertion failed, actual not greater than expected: ");
        print(file); print(":"); print_num_u(line, 10);
        print(" (Expected: > "); print_num(expected, 10);
        print(", Actual: "); print_num(actual, 10); println(")");
        print_color_set(color);
        #ifdef BREAK_ON_ASSERT_FAIL
        debug_break();
        #endif
    }
}

void _assert_gt_u32(uint32_t expected, uint32_t actual, int line, char* file) {
    if (!(actual > expected)) {
        uint8_t color = print_color_get();
        print_color_set(0x0C);
        print("Assertion failed, actual not greater than expected: ");
        print(file); print(":"); print_num_u(line, 10);
        print("(Expected: > "); print_num_u(expected, 10);
        print(", Actual: "); print_num_u(actual, 10); println(")");
        print_color_set(color);
        #ifdef BREAK_ON_ASSERT_FAIL
        debug_break();
        #endif
    }
}

void _assert_gt_u16(uint16_t expected, uint16_t actual, int line, char* file) {
    if (!(actual > expected)) {
        uint8_t color = print_color_get();
        print_color_set(0x0C);
        print("Assertion failed, actual not greater than expected: ");
        print(file); print(":"); print_num_u(line, 10);
        print("(Expected: > "); print_num_u(expected, 10);
        print(", Actual: "); print_num_u(actual, 10); println(")");
        print_color_set(color);
        #ifdef BREAK_ON_ASSERT_FAIL
        debug_break();
        #endif
    }
}

void _assert_gt_u8(uint8_t expected, uint8_t actual, int line, char* file) {
    if (!(actual > expected)) {
        uint8_t color = print_color_get();
        print_color_set(0x0C);
        print("Assertion failed, actual not greater than expected: ");
        print(file); print(":"); print_num_u(line, 10);
        print("(Expected: > "); print_num_u(expected, 10);
        print(", Actual: "); print_num_u(actual, 10); println(")");
        print_color_set(color);
        #ifdef BREAK_ON_ASSERT_FAIL
        debug_break();
        #endif
    }
}

void _assert_lt_i(int expected, int actual, int line, char* file) {
    if (!(actual < expected)) {
        uint8_t color = print_color_get();
        print_color_set(0x0C);
        print("Assertion failed, actual not less than expected: ");
        print(file); print(":"); print_num_u(line, 10);
        print(" (Expected: < "); print_num(expected, 10);
        print(", Actual: "); print_num(actual, 10); println(")");
        print_color_set(color);
        #ifdef BREAK_ON_ASSERT_FAIL
        debug_break();
        #endif
    }
}

void _assert_lt_u32(uint32_t expected, uint32_t actual, int line, char* file) {
    if (!(actual < expected)) {
        uint8_t color = print_color_get();
        print_color_set(0x0C);
        print("Assertion failed, actual not less than expected: ");
        print(file); print(":"); print_num_u(line, 10);
        print("(Expected: < "); print_num_u(expected, 10);
        print(", Actual: "); print_num_u(actual, 10); println(")");
        print_color_set(color);
        #ifdef BREAK_ON_ASSERT_FAIL
        debug_break();
        #endif
    }
}

void _assert_lt_u16(uint16_t expected, uint16_t actual, int line, char* file) {
    if (!(actual < expected)) {
        uint8_t color = print_color_get();
        print_color_set(0x0C);
        print("Assertion failed, actual not less than expected: ");
        print(file); print(":"); print_num_u(line, 10);
        print("(Expected: < "); print_num_u(expected, 10);
        print(", Actual: "); print_num_u(actual, 10); println(")");
        print_color_set(color);
        #ifdef BREAK_ON_ASSERT_FAIL
        debug_break();
        #endif
    }
}

void _assert_lt_u8(uint8_t expected, uint8_t actual, int line, char* file) {
    if (!(actual < expected)) {
        uint8_t color = print_color_get();
        print_color_set(0x0C);
        print("Assertion failed, actual not less than expected: ");
        print(file); print(":"); print_num_u(line, 10);
        print("(Expected: < "); print_num_u(expected, 10);
        print(", Actual: "); print_num_u(actual, 10); println(")");
        print_color_set(color);
        #ifdef BREAK_ON_ASSERT_FAIL
        debug_break();
        #endif
    }
}