
char* hello = "Hello from inside a process!";

void main() {
    __asm__ ("mov $0x00, %%eax; mov %0, %%esi; int $0x80" :: "r"(hello) : "eax", "esi");
}