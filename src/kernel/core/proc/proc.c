#include "proc.h"
#include "loader.h"
#include "core/mem/vmem.h"
#include "core/mem/palloc.h"
#include "util/print.h"
#include "util/panic.h"

int cur_pid = 0;
ProcessControlBlock processes[MAX_PROCESSES] = {0}; // Initialize all processes to 0

ProcessControlBlock* process_get(int pid) {
    if (pid < 0 || pid >= MAX_PROCESSES) {
        return 0;
    }
    return &processes[pid];
}

int process_exec(char* path) {
    // Load program into memory and setup page tables
    ProgramDescriptor program;
    int ret = load_program(path, &program);
    if (ret < 0) {
        return -1;
    }

    // Create process control block
    ProcessControlBlock* pcb;
    int i;
    for (i = 1; i < MAX_PROCESSES; i++) {
        pcb = &processes[i];
        if (pcb->state == PROC_STATE_STOPPED) {
            break;
        }
    }
    pcb->pid = i;
    pcb->state = PROC_STATE_RUNNING;
    pcb->page_directory = program.page_directory;
    pcb->parent_pid = 0;
    process_setup_stack(pcb);
    process_regs_init(pcb);

    pcb->regs.eip = (uint32_t)program.entry_point;
    destroy_program(&program); // We don't need the program anymore, free its used memory
    
    process_switch(pcb);
    return pcb->pid;
}

// When a process exits, we need to set its state to stopped, and free up resources.
// If this is the last process to close, panic to halt the system.
void process_exit() {
    ProcessControlBlock* pcb = &processes[cur_pid];
    pcb->state = PROC_STATE_STOPPED;
    process_destroy(pcb);
    // Find next process to run
    for (int i = 1; i < MAX_PROCESSES; i++) {
        if (processes[i].state == PROC_STATE_RUNNING) {
            process_switch(&processes[i]);
            return;
        }
    }
    // If we didn't find a process to run, we are the last process.
    panic("Last process exited, halting system.");
    // TODO - implement a proper shutdown sequence
}
void process_setup_stack(ProcessControlBlock* pcb) {
    palloc(0, PROCESS_STACK_SIZE, pcb->page_directory); // Allocate stack at 0x0000
    
    uint32_t temp_virt = 0xd0000000;
    uint32_t allocated_phys = vmem_entry_get(pcb->page_directory, 0) & ~(0xFFF); // Get physical address of the stack for the new process

    vmem_map(current_page_directory, (uint8_t*)allocated_phys, (uint8_t*)temp_virt, PROCESS_STACK_SIZE / PAGE_SIZE, PAGE_ENTRY_MASK_PRESENT | PAGE_ENTRY_MASK_READWRITE);
    vmem_load(current_page_directory); // Load page table we just setup so we can copy to allocated region
    
    pcb->stack_base = PROCESS_STACK_SIZE;
    pcb->stack_size = PROCESS_STACK_SIZE;
    // Push return address to the new processes stack. (via this temporary mapping)
    // When a program exits they with ret with this address.
    *((uint32_t*)(temp_virt + pcb->stack_base - 4)) = (uint32_t)process_exit;
    
    vmem_unmap(current_page_directory, (uint8_t*)temp_virt, PROCESS_STACK_SIZE / PAGE_SIZE); // Unmap the segment from the current page directory
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
    pcb->state = PROC_STATE_RUNNING;
    context_switch(pcb->pid);
}

void process_destroy(ProcessControlBlock* pcb) {
    pcb->state = PROC_STATE_STOPPED;
    malloc_cleanup(pcb->heap);
    vmem_pd_destroy(pcb->page_directory);
}

int dbg_counter = 0;

void process_yield() {
    // Find next running process, and switch to it.
    // We start at 1, since PID 0 stores the kernel's state before the first process is switched to.
    for (int i = 1; i < MAX_PROCESSES; i++) {
        if (processes[i].pid != cur_pid && processes[i].state == PROC_STATE_RUNNING) {
            process_switch(&processes[i]);
            return;
        }
    }
    // If we didn't find a process to switch to, just stay on the current process.
}

void process_kill(ProcessControlBlock* pcb) {
    process_destroy(pcb);
}

void proc_dbg() {
    int col = print_cursor_col_get();
    int row = print_cursor_row_get();

    print_cursor_set(0, 0);
    print("Cur PID: ");
    print_num_u(cur_pid, 10);

    print_cursor_set(row, col);
}
