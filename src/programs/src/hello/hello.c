
#include <stdbool.h>
#include "klib.h"

char* self = "This is process one!\n";

void print_self() {
    print(self);
}

void main() {
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