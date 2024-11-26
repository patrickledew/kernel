[org 0x10000] ; will be loaded immediately after boot sector and loaded by boot.s
[bits 32] ; this is code running after our switch to protected mode

section .kernel:

;; Entry point
kernel_init:
    ; First thing to do is set all the segment selectors to 0x10, which is the data segment we've set up
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax

setup_stack:
    ; Locate stack at 0x5000
    mov esp, 0xFFFF ; Stack pointer
    mov ebp, 0xFFFF ; Base pointer

kernel_hello:
    mov esi, kernel_string
    mov ch, 0x04
    call print
    mov esi, overflow_string
    call print
    jmp $
;; TODO:
;; 1. Figure out how to print stuff to the screen in protected mode (mostly done)
;; 2. Setup IDT and use to handle exceptions/timer interrupts
;; 3. Link C library, figure out how to compile and link C files

%include "src/print.s"

sti ; Enable interrupts    
kernel_string dw `Hello kernel!\nThis should appear on the next line.\n`, 0
overflow_string times 16 db "testing overflow "
db 0

times (0x200 - ($ - $$)) nop