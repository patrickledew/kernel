
char* hello = "Hello from inside a process!\n";
char* another = "Here's another one!";

void main() {
    __asm__ ("mov $0x00, %%eax; int $0x80" ::: "eax");
    __asm__ ("mov $0x01, %%eax; mov %0, %%ebx; int $0x80" :: "r"(hello) : "eax", "ebx");
    __asm__ ("mov $0x02, %%eax; int $0x80" :: "b"(hello), "c"(another) : "eax");
}