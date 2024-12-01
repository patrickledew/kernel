;; Boot sector
;; 512b
[org 0x7c00]
[bits 16]
; if using ELF, we can get debug symbols in gdb
; maybe we generate with elf, then objcopy everything to a binary
; how are orgs defined then?
; BUT ld only supports 32 bit

;;hgmmmmm
; how bout we use ELF, use that elf file for gdb, then ld

section .boot

load_kernel:
    mov di, string_loading_kernel
    call print_str
    ; kernel should be on disk immediately after this boot sector, from 0x200 onwards
    ; we want to load this to memory address 0x10000
    mov ah, 02h ; read sectors from drive - apparently 42h has more functionality
    mov al, 0x80 ; Copy up to 32 sectors (32 * 512 bytes)
    mov ch, 0 ; Cylinder 0
    mov cl, 2 ; 2nd sector, containing kernel
    mov dh, 0 ; Head 0
    mov dl, 0x80 ; Drive 0x80 (first hard drive)
    ; load to 0x10000
    mov bx, 0x1000
    mov es, bx 
    xor bx, bx
    int 13h

setup_gdt:
    mov di, string_setup_gdt
    call print_str
    cli
    ; Setup GDT
    xor eax, eax
    mov eax, gdt ; Get address of gdt
    mov [gdtr + 2], eax ; Store in base
    mov eax, gdt_end ; Get length of gdt
    sub eax, gdt
    mov [gdtr], ax ; Store in limit

    lgdt [gdtr] ; Load the GDT record

set_protected_mode:
    mov di, string_protected_mode
    call print_str
    ; Set protected mode bit
    mov eax, cr0
    or eax, 1
    mov cr0, eax
        
;; Picking up where u left off:
;; 1. gdb does not show correct address when long jumping
;; 2. there's something faulting in kernel_init.s, causing infinite restart

jump_kernel:
    jmp dword 0x08:0x10000 ; Jump using first GDT segment (offset 0x08), which is the kernel

;; UTILITIES

;; print_str
;; args:
;;    ds - null terminated string segment 
;;    di - null terminated string address
;; uses the current cursor pos
print_str:
    push ax
    push bx
    push cx
    push dx
    _print_char:
        ; get character
        mov ax, [di]
        ; if null terminator encountered, return
        cmp al, 0
        je _null
        cmp al, `\n` ; newline
        je _handle_newline
        ; print char
        mov ah, 0Ah
        mov bh, 0
        mov cx, 1
        int 10h
    _set_cursor_pos:
        __get_pos:
            mov ah, 03h
            mov bh, 0
            int 10h ; now dl = current col
        __get_size:
            mov ah, 0Fh
            int 10h ; now ah = num cols
        __inc_col:
            inc dl
        __overflow_check:
            cmp dl, ah
            je _handle_newline
        __set_pos:
            mov ah, 02h ; set position
            int 10h
    _repeat:
        ; increment dx and repeat
        inc di
        jmp _print_char

    ; special cases
    _handle_newline:
        ; get current pos
        mov ah, 03h
        mov bh, 0
        int 10h
        inc dh ; increment row
        xor dl, dl ; zero out column
        mov ah, 02h ; set position
        int 10h
        jmp _repeat
    _null:
        pop dx
        pop cx
        pop bx
        pop ax
        ret

;; GDT Info

gdtr:
    dw 0 ; limit of GDT record
    dd 0 ; base of GDT record

gdt:
    %include "src/boot/gdt.s"
gdt_end:

string_loading_kernel:
    dw `Loading kernel from disk image...\n`, 0
string_setup_gdt:
    dw `Setting up GDT...\n`, 0
string_protected_mode:
    dw `Turning on protected mode and jumping to kernel...\n`, 0

times (0x1FE - ($ - $$)) nop ;; pad for boot signature
bootsig:
    dw 0xAA55 ; Boot signature