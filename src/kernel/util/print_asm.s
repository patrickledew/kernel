section .text
;; Utilities for printing strings. Also keeps track of current cursor position.
global print
global print_asm

dw "Signature of print.s"
VIDEO_MEMORY equ 0xB8000
VIDEO_COLS equ 80
VIDEO_ROWS equ 25

;; print(char* string, uint8_t color_code)
;; Intended to be called from c, so stack arguments are used
;; esp + 0x00: pushed base pointer
;; esp + 0x04: ret addr
;; esp + 0x08: string
;; esp + 0x0C: color_code
print_old:
    push ebp
    mov ebp, esp

    mov esi, [esp+0x8]
    mov ch, [esp+0xC]
    call print_asm

    leave ;; eq to mov esp, ebp; pop ebp
    ret

;; print_asm
;; esi - address of null-terminated string
;; ch - color of text to print, first 3 bits = background, last 5 bits = foreground

print_asm:
    push esi ; Source of string
    push edi ; Destination address (in video memory)
    push eax ; Temp register
    push ebx ; Used for set_vga_crtc_reg
    push ecx ; ch: Text color param / cl: Character read from string 
    _set_pos:
        mov eax, VIDEO_COLS     ; EAX = num cols per row
        mul dword [cursor_row]   ; EAX = index to start of current row
        add eax, [cursor_col]   ; EAX = index to cursor position
        shl eax, 1              ; EAX = offset in text buffer
        add eax, VIDEO_MEMORY   ; EAX = address of character to modify
        mov edi, eax
    _print_char:
        mov cl, [esi] ; read character
        cmp cl, 0x0 ; check if null
        je _complete
        cmp cl, `\n` ; check if newline
        je _newline
        mov [edi], cl ; 
        mov [edi+1], ch ;
        ; move to next char
        inc dword [cursor_col]
        cmp dword [cursor_col], VIDEO_COLS
        jl _continue ; If overflowed available columns, go to a new line
    _newline:
        mov dword [cursor_col], 0
        inc dword [cursor_row]
        cmp dword [cursor_row], VIDEO_ROWS
        jl _continue ; If overflowed available rows, loop back from the beginning
    _wrap:
        mov dword [cursor_row], 0
    _continue:
        inc esi
        jmp _set_pos
    _complete:
    _update_cursor:
        ; Calculate offset
        mov ebx, edi ; Take address we are using to set VGA data
        sub ebx, VIDEO_MEMORY ; Subtract this from start of buffer
        shr ebx, 1 ; Divide by two to get the offset
        mov al, 0x0F ; Register 0x0F (Cursor Low Register)
        call set_vga_crtc_reg
        shr bx, 8 ;; Move bh -> bl
        dec al ; Register 0x0E (Cursor high register
        call set_vga_crtc_reg
    _done:
        pop ecx
        pop ebx
        pop eax
        pop edi
        pop esi
        ret

;; Write a value to VGA CRTC (CRT control) registers
;; This is used for controlling cursor position
;; al: Register code (e.g. 0x0F and 0x0E are cursor position low/high registers)
;; bl: Value to write to register
;;
;; VGA registers are accessed by providing an address to a register, then sending data to write to that register.:
;; Address - 3d4h
;; Data - 3d5h
set_vga_crtc_reg:
    push edx
    push eax
        mov dx, 0x03d4 ; 0x03d4 - CRTC address I/O port
        out dx, al     ; Set address to register code
        mov al, bl
        inc dx         ; 0x03d5 - CRTC data I/O port
        out dx, al     ; Set data of addressed register to al
    pop eax
    pop edx
    ret


cursor_col dd 0
cursor_row dd 0
        