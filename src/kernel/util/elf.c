#include "elf.h"
#include "logging.h"
#include "core/mem/memory.h"


int elf_parse_header(uint8_t* buffer, ElfHeader* header) {
    if (!buffer || !header) {
        return -1;
    }

    memcpy(buffer, (uint8_t*)header, sizeof(ElfHeader));

    // Check ELF magic number
    if (header->ident.magic[0] != 0x7F || header->ident.magic[1] != 'E' ||
        header->ident.magic[2] != 'L' || header->ident.magic[3] != 'F') {
        log_info("elf: Magic number invalid.");
        return -1;
    }
    return 0;
}

int elf_parse_program_header(uint8_t* buffer, ElfProgramHeader* header, uint32_t ph_offset) {
    if (!buffer || !header) {
        return -1;
    }

    memcpy(buffer + ph_offset, (uint8_t*)header, sizeof(ElfProgramHeader));

    return 0;
}