;; Boot sector
;; 512b
[org 0x7c00]
[bits 16]

section .boot

load_kernel:
    mov di, string_loading_kernel
    call print_str

    ; kernel should be on disk immediately after this boot sector, from 0x200 onwards
    ; we want to load this to memory address 0x10000
    
    load_chunk:
    mov ah, 42h ; Extended Read Sectors from Drive
    mov dl, 80h ; Load from first hard disk
    xor bx, bx
    ; specify ds:si for DAP
    mov ds, cx
    mov si, disk_dap
    int 13h
    jc disk_err
    mov ax, [disk_dap_segment]
    mov bx, [disk_dap_lba]
    add ax, 0x1000 ; We should load 0x10000 bytes higher in memory
    add bx, 0x80 ; 0x80 sectors
    mov [disk_dap_segment], ax
    mov [disk_dap_lba], bx
    sub ax, 0x2000 ; If segment > 0x2000, stop
    jl load_kernel_done
    jmp load_chunk
    load_kernel_done:

    mov di, string_disk_success
    call print_str

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
    ; We can't execute 32 bit code until we reload CS, which is done via a far jump
    ; We still need to move the kernel to the correct address, so we do that first
    jmp 0x08:move_kernel

[bits 32]
;; Now kernel is loaded at 0x10000-0x30000, we want to copy it to 0x100000
move_kernel:
    mov esi, 0x10000
    mov edi, 0x100000
    mov ecx, 0x8000 ; 0x8000 double words = 0x20000 bytes
    mov ax, 0x10
    mov es, ax
    mov ds, ax
    rep movsd

; Now we finally jump to the kernel_init code!
jump_kernel:
    jmp 0x100000 ; Jump using first GDT segment (offset 0x08), which is the kernel


disk_err:
    mov di, string_disk_error
    call print_str
disk_err_trap:
    jmp disk_err_trap


[bits 16]
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

;; DAP for reading in kernel
disk_dap:
    db 0x10 ; size of DAP
    db 0x00 ; reserved
    dw 0x0080 ; number of sectors to read (512 bytes * 0x80 = 0x10000 bytes)
    dw 0x0000 ; offset in segment to read to
disk_dap_segment:
    dw 0x1000 ; segment to read to (seg 0x1000 = byte 0x10000)
disk_dap_lba:
    dq 0x0000000000000001 ; LBA of kernel on disk (second sector)

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
string_disk_error:
    dw `Error reading kernel from disk.\n`, 0
string_disk_success:
    dw `Read kernel from disk successfully.\n`, 0

; times (0x1B8 - ($ - $$)) nop
;     dd 0x42424242 ;; Disk signature
;     dw 0x0000
; times (0x1BE - ($ - $$)) nop
; partition_entry_1: ;; KERNEL IMAGE PARTITION
;     db 0x80 ; Primary disk
    
;     ;; Start of partition
;     db 0x00 ; Head 0
;     db 0b00000010 ; Sector 2 (1-indexed)
;     db 0x00 ; Cylinder 0
;     db 0x7F ; partition type (must be non-zero)
    
;     ;; End of partition    
;     ;; We copied 0x80 sectors, so 0x81 is the last sector
;     db 0x02 ; Head 2
;     db 0x03 ; Sector 3 (1-indexed)
;     db 0x00 ; Cylinder 0

;     dd 0x01  ; LBA of start of partition (Sector 1, zero indexed)
;     dd 0x80  ; Sectors in partition
; partition_entry_2: ;; RESERVED FOR FILESYSTEM EXPERIMENTATION
;     db 0x80 ; Primary disk

;     ;; Start of partition - LBA 0x81
;     db 0x02 ; Head 2
;     db 0x04 ; Sector 4 (1-indexed)
;     db 0x00 ; Cylinder 0
;     db 0x01 ; partition type (FAT12)
    
;     ;; End of partition - LBA 0x882    
;     db 0x02 ; Head 2
;     db 0x25 ; Sector 37 (1-indexed)
;     db 0x02 ; Cylinder 2
;     dd 0x81  ; LBA of start of partition (Sector 1, zero indexed)
;     dd 0x800  ; Sectors in partition
    
times (0x1FE - ($ - $$)) db 0x00 ;; pad for boot signature
bootsig:
    dw 0xAA55 ; Boot signature