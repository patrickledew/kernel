section .kernel_init
[bits 32] ; this is code running after our switch to protected mode

global _start

extern kmain

;; Entry point
_start:
    ; First thing to do is set all the segment selectors to 0x10, which is the data segment we've set up
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax

setup_stack:
    ; Locate stack at 0x5000
    mov esp, 0xFFFF ; Stack pointer
    mov ebp, 0xFFFF ; Base pointer
call_main:
    jmp kmain