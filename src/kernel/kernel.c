// C Entry point for kernel. Invoked from kernel_init.s.

extern void print(char* string, char color_code);

void kmain() {
    print("Hello kernel!", 0x0F);
    for (;;){}
}