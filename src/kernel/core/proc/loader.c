#include "loader.h"
#include "util/elf.h"
#include "core/mem/alloc.h"
#include "core/mem/vmem.h"
#include "core/fs/fs.h"
#include "util/logging.h"
#include "util/assert.h"

void loader_test(char* path) {
    int fd = open(path, 0);
    if (fd == -1) {
        log_error("loader_test: File not found.");
        return;
    }
    ProgramDescriptor program;
    program.page_directory = vmem_pd_create(); // Creates a fresh page directory

    // Read in file
    int size = fsize(fd);
    program.program_memory = (uint8_t*)alloc(size);
    read(fd, program.program_memory, size);

    // Parse ELF header
    ElfHeader header;
    if(elf_parse_header(program.program_memory, &header) == -1) {
        log_error("loader_test: Invalid ELF header.");
        return;
    }
    program.entry_point = (void (*)())header.entry;

    ElfProgramHeader ph;
    for (int ph_off = header.ph_offset; ph_off < header.ph_entry_count*header.ph_entry_size; ph_off += header.ph_entry_size) {
        elf_parse_program_header(program.program_memory, &ph, ph_off);
        if (ph.type == ELF_PT_LOAD) {
            load_segment(&program, &ph);
        }
    }
    
    vmem_load(program.page_directory);
    log_number_u("Loaded process, entry point", program.entry_point, 16);

    program.entry_point();
}

void load_segment(ProgramDescriptor* program, ElfProgramHeader* ph) {
    // First, set up page tables to map the segment
    assert_u32(ph->align, 0x1000); // Ensure alignment is 4k, because thats how our paging is set up
    uint32_t num_pages = (ph->mem_size + 0xFFF) / 0x1000;
    uint32_t flags = PAGE_ENTRY_MASK_PRESENT | PAGE_ENTRY_MASK_READWRITE;
    vmem_map(program->page_directory, KADDR_TO_PADDR(program->program_memory + ph->offset), (uint8_t*)ph->v_addr, num_pages, flags);

}
