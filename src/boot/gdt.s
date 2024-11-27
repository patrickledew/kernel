;; Global Descriptor Table
;; Contains segment descriptors, which are data structures that specify the location, size,
;; and other information about memory segments. These can be accessed via jmp instructions, e.g.
;; jmp 3:0x1234, which will jump 


;; For simplicity, this uses a flat address space. Both code and data segments for the kernel cover Page 0 - Page 0x1FFFFF.

;; Null descriptor
entry_null:
    times 8 db 0 ; 8 null bytes
entry_1: ; Kernel code segment
    _1_limit_low   dw 0xFFFF    ; Limit - maximum addressable unit, this leaves 64KiB for kernel code
    _1_base_low    dw 0x0000    ; LSBs of base address

    _1_base_mid    db 0x00      ; Next MSB of base
    _1_access      db 10011010b ; Present, Privilege (00 - Kernel), Descriptor Type (1 - Code/Data), Executable (1), Conforming (0), Readable (1), Accessed (1)
    _1_flags_limit db 11011111b ; First 3 bits are flags (Granularity: Page, Size: 32bit, Long mode: No), rest are MSBs of limit
    _1_base_high   db 0x00      ; MSB of base
entry_2: ; Kernel data segment
    _2_limit_low   dw 0xFFFF    ; Limit - maximum addressable unit
    _2_base_low    dw 0x0000    ; LSBs of base address

    _2_base_mid    db 0x00      ; Next MSB of base
    _2_access      db 10010010b ; Present, Privilege (00 - Kernel), Descriptor Type (1 - Code/Data), Executable (0), Conforming (0), Readable (1), Accessed (1)
    _2_flags_limit db 11011111b ; First 3 bits are flags (Granularity: Page, Size: 32bit, Long mode: No), rest are MSBs of limit
    _2_base_high   db 0x00      ; MSB of base