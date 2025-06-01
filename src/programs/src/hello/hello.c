
#include <stdbool.h>
#include "klib.h"

char* self = "This is process one!\n";

void print_self() {
    print(self);
}

void test_yield() {
    exec("/SECOND.ELF");

    while(true) {
        print_self();
        char* str = "Test char: X";
        for (unsigned char c = '0'; c < '9'; c++) {
            str[11] = c;
            println(str);
        }
    
        yield();
    }
}

void main() {
    print_self();
    println("HELLO.ELF Malloc Test");

    char* ptr = malloc(0x1000);
    if (ptr == 0) {
        println("Malloc failed!");
        exit();
    }
    println("Malloc succeeded!");
    for (int i = 0; i < 0x1000; i++) {
        ptr[i] = i % 256;
    }
    println("Memory initialized!");
    println((char*)&ptr[1]);
    int ret = free(ptr);
    if (ret != 0) {
        println("Free failed!");
    } else {
        println("Free succeeded!");
    }

    println("Using after free:");
    println((char*)&ptr[1]); // This should not be used after free, but we do it anyway to test the behavior.
 
    // Next test: make sure when multiple processes are allocating memory,
    // they dont overwrite each other's memory.

    ptr = malloc(0x1000);
    if (ptr == 0) {
        println("Second malloc failed!");
        exit();
    }
    println("Second malloc succeeded!");


    exec("/SECOND.ELF");

    while (true) {
        for (int i = 0; i < 0x1000; i++) {
            ptr[i] = 'A'; // Fill with '1's
        }
        ptr[50] = '\0'; // Set last byte to 0
        yield();
        println(ptr);
    }
 
    println("Exiting process one.");
    exit();
}