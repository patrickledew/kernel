int main() {
    __asm__ volatile("int $0x00"); // Divide by zero interrupt - should let us know we're executing this code!
    return 0;
}