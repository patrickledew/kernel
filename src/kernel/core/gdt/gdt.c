#include "gdt.h"

gdt_segment_descriptor gdt[3];

gdt_descriptor gdtr;


void gdt_init() {
    // Zero out null entry
    gdt[0].access = 0;
    gdt[0].base_high = 0;
    gdt[0].base_low = 0;
    gdt[0].base_mid = 0;
    gdt[0].flags_limit = 0;
    gdt[0].limit_low = 0;

    // Kernel code segment
    gdt[1].access = 0b10011010; // Present, Privilege (00 - Kernel), Descriptor Type (1 - Code/Data), Executable (1), Conforming (0), Readable (1), Accessed (1)
    gdt[1].base_high = 0;
    gdt[1].base_low = 0;
    gdt[1].base_mid = 0;
    gdt[1].flags_limit = 0b11011111; // First 3 bits are flags (Granularity: Page, Size: 32bit, Long mode: No), rest are MSBs of limit
    gdt[1].limit_low = 0xFFFF;

    // Kernel data segment
    gdt[2].access = 0b10010010; // ; Present, Privilege (00 - Kernel), Descriptor Type (1 - Code/Data), Executable (0), Conforming (0), Readable (1), Accessed (1)
    gdt[2].base_high = 0;
    gdt[2].base_low = 0;
    gdt[2].base_mid = 0;
    gdt[2].flags_limit = 0b11011111; // First 3 bits are flags (Granularity: Page, Size: 32bit, Long mode: No), rest are MSBs of limit
    gdt[2].limit_low = 0xFFFF;

    // Setup GDTR    
    gdtr.offset = gdt;
    gdtr.size = sizeof(gdt_segment_descriptor) * 3 - 1;
    gdt_load(&gdtr);
}

void gdt_load(gdt_descriptor* gdtr) {
    __asm__("lgdt (%%eax)\n\t"
            "ljmp $0x08, $done\n\t" // Need long jump to load code segment reg
            "done:" :: "a"(gdtr));
}