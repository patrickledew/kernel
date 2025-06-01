#include "klib.h"
#include <stdbool.h>
char* self = "This is process two!\n";

void print_self() {
    print(self);
}

void main() {
    print_self();
    char* ptr = malloc(0x1000);
    if (ptr == 0) {
        println("SECOND.ELF malloc failed!");
        exit();
    }
    println("SECOND.ELF malloc succeeded!");

    while (true) {
        for (int i = 0; i < 0x1000; i++) {
            ptr[i] = 'B'; // Fill with '2's
        }
        // *ptr = "Hello world";
        ptr[5] = '\0'; // Set last byte to 0
        yield();
        println(ptr);
    }
    exit();
}