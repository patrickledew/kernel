;; Utilities for printing strings. Also keeps track of current cursor position.

VIDEO_MEMORY equ 0xB8000
VIDEO_COLS equ 80
VIDEO_ROWS equ 25

;; print
;; esi - address of null-terminated string
;; ch - color of text to print, first 3 bits = background, last 5 bits = foreground
print:
    push esi ; Source of string
    push edi ; Destination address (in video memory)
    push edx
    push ecx ; ch: Text color param / cl: Character read from string 
    push eax ; Temp register
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
    _continue:
        inc esi
        jmp _set_pos
    _complete:
    _update_cursor:
        ; Calculate offset
        mov ecx, edi ; Take address we are using to set VGA data
        sub ecx, VIDEO_MEMORY ; Subtract this from start of buffer
        shr ecx, 1 ; Divide by two to get the offset

        ;; VGA registers are accessed by providing an address through the address register and data registers:
        ;; Address - 3d4h
        ;; Data - 3d5h
        
        ; First, we send the value of the register we want to access (0x0F = Cursor low register)
        mov dx, 0x03d4 ; Address
        mov al, 0x0F ; Low register
        out dx, al

        ; We then send 
        inc dx ; Now 0x3d5 (data)
        mov al, cl ; Send low byte of offset
        out dx, al

        dec dx ; Now 0x3d4 (Address)
        mov al, 0x0E ; High register
        out dx, al
        
        inc dx ; Now 0x3d5 (data)
        mov al, ch ; Send high byte of offset
        out dx, al
    _done:
        pop eax
        pop ecx
        pop edx
        pop edi
        pop esi
        ret

cursor_col dd 0
cursor_row dd 0
        