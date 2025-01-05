section .text

global vmem_enable
global vmem_setup_stack

extern k_page_directory
extern vmain

vmem_enable:
    push eax
    mov eax, k_page_directory ; Load CR3 with page directory
    mov cr3, eax
    mov eax, cr0            
    or eax, 0x80000000        ; Set PG bit in CR0
    mov cr0, eax
    jmp vmem_enable_done + 0xC0000000
vmem_enable_done:
    ; Return address will not have correct offset, need to add 0xC0000000 to it
    mov eax, [esp+4]
    add eax, 0xC0000000
    mov [esp+4], eax
    pop eax
    ret

; Update kernel stack to point to where we've mapped our kernel
vmem_setup_stack:
    push eax
    mov eax, esp
    add eax, 0xC0000000
    mov esp, eax
    mov eax, ebp
    add eax, 0xC0000000
    mov ebp, eax
    pop eax
    ret