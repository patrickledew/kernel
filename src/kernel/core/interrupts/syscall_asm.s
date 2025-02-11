[bits 32]
global syscall_handler
extern syscall_table
section .text

;; Seems this needs to be written in asm, since GCC automatically preserves registers.
;; We want to (intentionally) set eax to act as the return code, so we can't write a normal C function.
syscall_handler:
    push edx ; Push registers to stack
    push esi
    push edi
    push ebp
    push esp

    push ecx ; ecx and ebx are params if applicable
    push ebx
    ; Get the syscall entry from eax
    ; Routine stored in ebx, num args stored in ecx
    and eax, 0xFF ; Only the last 8 bits are the syscall number
    mov dword ebx, [syscall_table + eax*8]
    mov dword ecx, [syscall_table + eax*8 + 4]
    ; do syscall
    call syscall_setup_args

    pop ebx
    pop ecx

    pop esp
    pop ebp
    pop edi
    pop esi
    pop edx
syscall_done:
    iret ;; When returning, eax will have the return value of the syscall

syscall_setup_args:
    ; ebx = routine, ecx = num args
    ; Start new stack frame for routine
    push ebp ; Now first arg = esp+8, second = esp+12
    mov ebp, esp
    ; now ebp = old ebp, ebp + 4 = ret addr, ebp + 8 = param1, ebp + 12 = param2

push_args:
    ;; Push arguments based on number of arguments
    mov edx, ecx
    sub edx, 2
    jz two_args
    mov edx, ecx
    sub edx, 1
    jz one_arg
    jmp syscall_invoke
two_args:
    push dword [ebp + 12] ; param 2
one_arg:
    push dword [ebp + 8] ; param 1
syscall_invoke:
    call ebx
    ; Remove stack frame
syscall_return:
    mov esp, ebp
    pop ebp
    ret
