[bits 32]

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
    mov ebx, 92
    mul ebx ; eax = pid * sizeof(ProcessControlBlock)
    pop ebx
    add eax, processes ; eax = &processes[pid] = &processes[pid].regs
    mov [eax + 4], ebx ; Save ebx
    mov [eax + 8], ecx ; Save ecx
    mov [eax + 12], edx ; Save edx
    mov [eax + 16], esi ; Save esi
    mov [eax + 20], edi ; Save edi
    mov [eax + 24], ebp ; Save ebp
    add esp, 4
    mov [eax + 28], esp ; Save esp+4 since we want it how it was before we pushed eax
    sub esp, 4
    mov dword [eax + 32], context_return ; When we next restore this process, it will end up in the context_return block.
    ; mov [eax + 36], eflags ; Save eflags
    mov [eax + 40], cs ; Save cs
    mov [eax + 44], ds ; Save ds
    mov [eax + 48], es ; Save es
    mov [eax + 52], fs ; Save fs
    mov [eax + 56], gs ; Save gs
    mov [eax + 60], ss ; Save ss
    mov ebx, eax
    ; we didn't save eax yet, but we pushed it to the stack. pop and save it
    pop eax
    mov [eax], eax

set_pid:
    mov eax, [esp+4]
    mov [cur_pid], eax ; Update cur_pid to the new process
load_new_regs:
    ; Load the next process context from the pid passed in the stack
    ; ebx = sizeof(ProcessControlBlock)
    mov ebx, 92
    mul ebx ; eax = pid * sizeof(ProcessControlBlock)
    add eax, processes ; eax = &processes[pid] = &processes[pid].regs
    ; ebx = page directory
    mov ebx, [eax + 72]
    ; Convert to physical address
    sub ebx, 0xC0000000
    add ebx, 0x100000
    mov cr3, ebx ; Load page directory
    mov ebx, [eax + 4] ; Load ebx
    mov ecx, [eax + 8] ; Load ecx
    mov edx, [eax + 12] ; Load edx
    mov esi, [eax + 16] ; Load esi
    mov edi, [eax + 20] ; Load edi
    mov ebp, [eax + 24] ; Load ebp
    mov esp, [eax + 28] ; Load esp
    ; mov eflags, [eax + 36] ; Load eflags
    mov ds, [eax + 44] ; Load ds
    mov es, [eax + 48] ; Load es
    mov fs, [eax + 52] ; Load fs
    mov gs, [eax + 56] ; Load gs
    mov ss, [eax + 60] ; Load ss

    ; Note: now we are using the processes stack
    push dword [eax+40] ; Push processes CS onto stack
    push dword [eax+32] ; Push processes eip on to stack

    ; Finally restore eax, since we couldn't do that before
    mov eax, [eax]
    
    sti
    retf ; Far jump to the process. This should reload CS

context_return:
    ret
    jmp $
    ; TODO what do we need to do in order to launch the process
    ; TBD:
    ; - Args and how we get them
    ; - Do we use TSS or not (hardware context switching)
    ; - How to jump to process
    
    ; 1. Save the current process context based on current PID
    ; 2. Load the next process context based on regs passed