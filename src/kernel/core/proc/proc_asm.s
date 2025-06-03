[bits 32]

%define PCB_SIZE 100 ; Size of ProcessControlBlock in bytes

%define PCB_REGS_SIZE 64 ; Size of Registers struct in bytes
%define PCB_REG_EAX 0
%define PCB_REG_EBX 4
%define PCB_REG_ECX 8
%define PCB_REG_EDX 12
%define PCB_REG_ESI 16
%define PCB_REG_EDI 20
%define PCB_REG_EBP 24
%define PCB_REG_ESP 28
%define PCB_REG_EIP 32
%define PCB_REG_EFLAGS 36
%define PCB_REG_CS 40
%define PCB_REG_DS 44
%define PCB_REG_ES 48
%define PCB_REG_FS 52
%define PCB_REG_GS 56
%define PCB_REG_SS 60

%define PCB_STATE 64 ; Offset for ProcessState in ProcessControlBlock
%define PCB_PID 68 ; Offset for pid in ProcessControlBlock
%define PCB_PAGE_DIR 72 ; Offset for page directory in ProcessControlBlock

section .text
global context_switch
extern cur_pid
extern processes
;;;;
;typedef struct {
;    Registers regs; 0
;    uint32_t pid; 64 
;    ProcessState state; 68
;    uint32_t* page_directory; 72
;    uint32_t priority; 76      // Process priority level
;    uint32_t stack_base; 80    // Stack pointer
;    uint32_t stack_size; 84   // Size of process stack
;    uint32_t status;  88      // Exit status
;} ProcessControlBlock; 92
;;;;
;typedef struct {
;    uint32_t eax;
;    uint32_t ebx;
;    uint32_t ecx;
;    uint32_t edx;
;    uint32_t esi;
;    uint32_t edi;
;    uint32_t ebp;
;    uint32_t esp;
;    uint32_t eip;
;    uint32_t eflags;
;    uint32_t cs;
;    uint32_t ds;
;    uint32_t es;
;    uint32_t fs;
;    uint32_t gs;
;    uint32_t ss;
;} Registers;
;;;;
;;;; Stack:
; $esp + 0: Ret addr
; $esp + 4: int pid
;;;; Other symbols:
; cur_pid: current pid. Used as index into table
; processes: ProcessControlBlock[] - process table
context_switch:
     ; For some reason, we are getting page faults when the timer interrupt
     ; occurs during a context switch. Disable interrupts so this doesnt happen.
    cli
store_old_regs:
    ; Save the current process context to the process table according to current pid
    push eax ; will be using this to get entry in process table
    push ebx 
    mov eax, [cur_pid]
    mov ebx, PCB_SIZE
    mul ebx ; eax = pid * sizeof(ProcessControlBlock)
    pop ebx
    add eax, processes ; eax = &processes[pid] = &processes[pid].regs
    mov [eax + PCB_REG_EBX], ebx ; Save ebx
    mov [eax + PCB_REG_ECX], ecx ; Save ecx
    mov [eax + PCB_REG_EDX], edx ; Save edx
    mov [eax + PCB_REG_ESI], esi ; Save esi
    mov [eax + PCB_REG_EDI], edi ; Save edi
    mov [eax + PCB_REG_EBP], ebp ; Save ebp
_dbg:
    add esp, 4
    mov [eax + PCB_REG_ESP], esp ; Save esp+4 since we want it how it was before we pushed eax
    sub esp, 4
    mov dword [eax + PCB_REG_EIP], context_return ; When we next restore this process, it will end up in the context_return block.
    ; mov [eax + PCB_REG_EFLAGS], eflags ; Save eflags
    mov [eax + PCB_REG_CS], cs ; Save cs
    mov [eax + PCB_REG_DS], ds ; Save ds
    mov [eax + PCB_REG_ES], es ; Save es
    mov [eax + PCB_REG_FS], fs ; Save fs
    mov [eax + PCB_REG_GS], gs ; Save gs
    mov [eax + PCB_REG_SS], ss ; Save ss
    mov ebx, eax
    ; we didn't save eax yet, but we pushed it to the stack. pop and save it
    pop eax
    mov [ebx + PCB_REG_EAX], eax

set_pid:
    mov eax, [esp+4]
    mov [cur_pid], eax ; Update cur_pid to the new process
load_new_regs:
    ; Load the next process context from the pid passed in the stack
    ; ebx = sizeof(ProcessControlBlock)
    mov ebx, PCB_SIZE
    mul ebx ; eax = pid * sizeof(ProcessControlBlock)
    add eax, processes ; eax = &processes[pid] = &processes[pid].regs
    ; ebx = page directory
    mov ebx, [eax + PCB_PAGE_DIR]
    ; Convert to physical address
    sub ebx, 0xC0000000
    add ebx, 0x100000
    mov cr3, ebx ; Load page directory
    mov ebx, [eax + PCB_REG_EBX] ; Load ebx
    mov ecx, [eax + PCB_REG_ECX] ; Load ecx
    mov edx, [eax + PCB_REG_EDX] ; Load edx
    mov esi, [eax + PCB_REG_ESI] ; Load esi
    mov edi, [eax + PCB_REG_EDI] ; Load edi
    mov ebp, [eax + PCB_REG_EBP] ; Load ebp
    mov esp, [eax + PCB_REG_ESP] ; Load esp
    ; mov eflags, [eax + PCB_REG_EFLAGS] ; Load eflags
    mov ds, [eax + PCB_REG_DS] ; Load ds
    mov es, [eax + PCB_REG_ES] ; Load es
    mov fs, [eax + PCB_REG_FS] ; Load fs
    mov gs, [eax + PCB_REG_GS] ; Load gs
    mov ss, [eax + PCB_REG_SS] ; Load ss

    ; Note: now we are using the processes stack
    push dword [eax + PCB_REG_CS] ; Push processes CS onto stack
    push dword [eax + PCB_REG_EIP] ; Push processes eip on to stack

    ; Finally restore eax, since we couldn't do that before
    mov eax, [eax + PCB_REG_EAX]
    
    sti
    retf ; Far jump to the process. This should reload CS

context_return:
    ret
    jmp $