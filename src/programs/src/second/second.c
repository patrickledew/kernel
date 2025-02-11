#include "klib.h"

char* self = "This is process two!\n";

void print_self() {
    print(self);
}

void main() {
    while(1) {
        print_self();
        yield();
    }
}