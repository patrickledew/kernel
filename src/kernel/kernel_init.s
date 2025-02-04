[bits 32] ; this is code running after our switch to protected mode
section .kernel_init

global _start
global _KERNEL_PAGE_DIRECTORY
global _KERNEL_PAGE_TABLE

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

setup_temp_stack:
    ; Locate stack at 0xFFFF
    mov esp, 0xFFFF ; Stack pointer
    mov ebp, 0xFFFF ; Base pointer

setup_paging:
    populate_page_tables:
    ;; First entry is the identity mapping for 0x0-0x400000
    ;; This is needed so our code can continue running from the same address until we jump to the higher half kernel
    jmp populate_identity
    
    ; Fill page table at `edi` with flat map of addresses starting at `eax`.
    fill_pt:
    mov ecx, eax ; ecx = original target start addr
    or eax, 3
    fill_pt_loop:
    mov [edi], eax
    add eax, 0x1000
    add edi, 4
    ; check if eax is past highest value
    mov ebx, eax ; ebx has absolute addr
    sub ebx, ecx ; ebx has relative offset
    sub ebx, 0x400000 ; size of mapped region (1024 pages)
    jl fill_pt_loop
    ret
    
    populate_identity:
    mov edi, identity
    xor eax, eax
    call fill_pt

    populate_kernel:
    mov edi, init_page_table
    mov eax, 0x100000 ; Start address of kernel in physical memory
    call fill_pt
    ;; Also map the last page to the VGA text mode buffer, so we can still write stuff to the screen
    mov edi, init_page_table + 0xFFC
    mov [edi], dword 0xB8003

    populate_page_dir:
    mov edi, init_page_directory
    mov eax, identity
    or eax, 3 ;; add permission bits
    mov [edi], eax
    ;; Next, we need to map the kernel (0x100000-0x500000) to 0xC0000000
    add edi, 0xC00 
    mov eax, init_page_table
    or eax, 3 ;; add permission bits
    mov [edi], eax
    
    enable_paging:
        mov eax, init_page_directory
        mov cr3, eax
        mov eax, cr0            
        or eax, 0x80000000        ; Set PG bit in CR0
        mov cr0, eax
setup_kernel_stack:
    mov ebp, 0xc03f0000 ; Stack located at 0x4f0000 physical, 0xc03f00000 virtual
    mov esp, ebp        ; this puts the stack high up in the kernel's reserved memory,
                        ; so it can grow down as much as it needs to
    
call_main:
    jmp kmain

;; Allocate space for page directory and page tables. Kernel page table will be reused.
align 4096

_KERNEL_PAGE_DIRECTORY:
init_page_directory:
    times 1024 dd 0 ; 1024 32-bit entries

_KERNEL_PAGE_TABLE:
init_page_table:
    times 1024 dd 0

identity:
    times 1024 dd 0