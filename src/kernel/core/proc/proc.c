#include "proc.h"
#include "loader.h"
#include "core/mem/vmem.h"
#include "core/mem/palloc.h"

int pid_counter = 1; // Start at 1 so first process is PID=1
int cur_pid = 0;
ProcessControlBlock processes[100];

int exec(char* path) {
    // Load program into memory and setup page tables
    ProgramDescriptor program;
    load_program(path, &program);

    // Create process control block
    ProcessControlBlock* pcb = &processes[pid_counter];
    pcb->pid = pid_counter++;
    pcb->state = PROC_STATE_RUNNING;
    pcb->page_directory = program.page_directory;
    pcb->priority = 0; // Unused
    process_setup_stack(pcb);
    process_regs_init(pcb);
    pcb->regs.eip = (uint32_t)program.entry_point;
    destroy_program(&program); // We don't need the program anymore, free its used memory
    
    process_switch(pcb);
    return pcb->pid;
}

// When a process exits, we need to set its state to stopped, and free up resources.
// If this is the last process to close, we should loop infinitely.
void process_exit() {
    ProcessControlBlock* pcb = &processes[cur_pid];
    pcb->state = PROC_STATE_STOPPED;
    process_destroy(pcb);
    pid_counter--;
    if (pid_counter == 1) {
        while(1) {}
    } else {
        // TODD - implement process scheduling
        while(1) {}
    }
}
void process_setup_stack(ProcessControlBlock* pcb) {
    palloc(0, PROCESS_STACK_SIZE, pcb->page_directory); // Allocate stack at 0x0000
    
    uint32_t temp_virt = 0xd0000000;
    uint32_t allocated_phys = vmem_entry_get(pcb->page_directory, 0) & ~(0xFFF); // Get physical address of the stack for the new process

    vmem_map(current_page_directory, allocated_phys, temp_virt, PROCESS_STACK_SIZE / PAGE_SIZE, PAGE_ENTRY_MASK_PRESENT | PAGE_ENTRY_MASK_READWRITE);
    vmem_load(current_page_directory); // Load page table we just setup so we can copy to allocated region
    
    pcb->stack_base = PROCESS_STACK_SIZE;
    pcb->stack_size = PROCESS_STACK_SIZE;
    // Push return address to the new processes stack. (via this temporary mapping)
    // When a program exits they with ret with this address.
    *((uint32_t*)(temp_virt + pcb->stack_base - 4)) = (uint32_t)process_exit;
    
    vmem_unmap(current_page_directory, temp_virt, PROCESS_STACK_SIZE / PAGE_SIZE); // Unmap the segment from the current page directory
    vmem_load(current_page_directory); // Unload the temporary mapping

    pcb->regs.esp = pcb->stack_base - 4;
    pcb->regs.ebp = pcb->stack_base;
}
void process_regs_init(ProcessControlBlock* pcb) {
     // Stack will be set to 0x000-0xFFF
    pcb->regs.eax = 0xBABABEEF;
    pcb->regs.ebx = 0xB0B0BEEF;
    pcb->regs.ecx = 0xBABABEEF;
    pcb->regs.edx = 0xB0B0BEEF;
    pcb->regs.esi = 0xBABABEEF;
    pcb->regs.edi = 0xB0B0BEEF;
    pcb->regs.cs = 0x08;
    pcb->regs.ds = 0x10;
    pcb->regs.es = 0x10;
    pcb->regs.fs = 0x10;
    pcb->regs.gs = 0x10;
    pcb->regs.ss = 0x10;
    pcb->regs.eflags = 0x202; // just trust me bro -copilot
}

void process_switch(ProcessControlBlock* pcb) {
    current_page_directory = pcb->page_directory;
    context_switch(pcb->pid);
}

void process_destroy(ProcessControlBlock* pcb) {
    pcb->state = PROC_STATE_STOPPED;
    vmem_pd_destroy(pcb->page_directory);
}

int dbg_counter = 0;

void process_yield() {
    // Find next running process, and switch to it.
    // TODO implement actual process scheduling
    // We start at 1, since PID 0 stores the kernel's state before the first process is switched to.
    for (int i = 1; i < 100; i++) {
        if (processes[i].pid != cur_pid && processes[i].state == PROC_STATE_RUNNING) {
            process_switch(&processes[i]);
            dbg_counter++;
            return;
        }
    }
}