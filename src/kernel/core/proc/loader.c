#include "loader.h"
#include "util/elf.h"
#include "core/mem/alloc.h"
#include "core/mem/vmem.h"
#include "core/fs/fs.h"
#include "core/mem/palloc.h"
#include "core/mem/memory.h"
#include "util/logging.h"
#include "util/assert.h"

void loader_test(char* path) {
    ProgramDescriptor program;
    load_program(path, &program);
    program.entry_point();
}

// Loads an ELF file into memory and allocates memory for each segment in the executable.
// Returns 0 on success, -1 on failure.
int load_program(char* path, ProgramDescriptor* program) {
    int fd = open(path, 0);
    if (fd == -1) {
        return -1;
    }

    // Creates a fresh page directory
    program->page_directory = vmem_pd_create();

    // Read in binary
    program->size = (uint32_t)fsize(fd);
    program->data = (uint8_t*)alloc(program->size);
    read(fd, program->data, program->size);
    close(fd);

    // Parse ELF header
    ElfHeader header;
    if(elf_parse_header(program->data, &header) == -1) {
        log_error("loader_test: Invalid ELF header.");
        return -1;
    }

    // Specify entry point
    program->entry_point = (void (*)())header.entry;

    // Parse program headers and allocate memory for the process (segments)
    ElfProgramHeader ph;
    for (int ph_off = header.ph_offset; ph_off < header.ph_entry_count*header.ph_entry_size; ph_off += header.ph_entry_size) {
        elf_parse_program_header(program->data, &ph, ph_off);
        if (ph.type == ELF_PT_LOAD) {
            load_segment(program, &ph);
        }
    }
    return 0;
}

void load_segment(ProgramDescriptor* program, ElfProgramHeader* ph) {
    assert_u32(ph->align, 0x1000); // Ensure alignment is 4k, because thats how our paging is set up
    uint32_t num_pages = (ph->mem_size + 0xFFF) / 0x1000;
    palloc((uint8_t*)ph->v_addr, num_pages, program->page_directory);

    // We have to use a temporary mapping in order to copy the data into the allocated region.
    // Since the rest of the page table (including stack mapping) has not been setup yet,
    // we will page fault if we switch to it now.

    // TODO honestly think there may be a bug if we ever load multiple pages
    // palloc does not always give contiguous memory regions
    // so we'd need to check every PT entry and map it to temporary region
    // basically just do it page by page
    uint32_t allocated_phys = vmem_entry_get((uint32_t*)program->page_directory, (uint8_t*)ph->v_addr) & ~(0xFFF); // Get physical address of this segment
    uint32_t temp_virt = 0xd0000000;
    vmem_map(current_page_directory, (uint8_t*)allocated_phys, (uint8_t*)temp_virt, num_pages, PAGE_ENTRY_MASK_PRESENT | PAGE_ENTRY_MASK_READWRITE);
    vmem_load(current_page_directory); // Load page table we just setup so we can copy to allocated region
    
    memcpy(program->data + ph->offset, (uint8_t*)temp_virt, ph->file_size);
    
    vmem_unmap(current_page_directory, (uint8_t*)temp_virt, num_pages); // Unmap the segment from the current page directoryn
    vmem_load(current_page_directory); // Load page table we just setup so we can copy to allocated region

}


// Frees memory used to store raw program binary - does not free page tables
void destroy_program(ProgramDescriptor* program) {
    free(program->data);
}