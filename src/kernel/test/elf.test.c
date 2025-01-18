#include "elf.test.h"
#include "util/elf.h"
#include "core/mem/alloc.h"
#include "core/fs/fs.h"
#include "util/assert.h"
#include "util/logging.h"

void elf_test() {
    int fd = open("/HELLO.ELF", 0);

    int size = fsize(fd);
    assert_not_i(size, -1);

    uint8_t* buffer = (uint8_t*)alloc(size);
    read(fd, buffer, size);

    ElfHeader header;
    elf_parse_header(buffer, &header);

    log_info("elf: Parsed ELF header.");
    // Log ELF header information
    log_number_u("elf: ident: Class", header.ident.class, 16);
    log_number_u("elf: ident: Endianness", header.ident.endianness, 16);
    log_number_u("elf: ident: Version", header.ident.version, 16);
    log_number_u("elf: ident: ABI", header.ident.os_abi, 16);
    log_number_u("elf: ident: ABI Ver", header.ident.abi_version, 16);

    log_number_u("elf: Type", header.type, 16);
    log_number_u("elf: ISA", header.machine, 16);
    log_number_u("elf: Version", header.version, 16);
    log_number_u("elf: Entry Point", header.entry, 16);
    log_number_u("elf: PH Table", header.ph_offset, 16);
    log_number_u("elf: PH Entries", header.ph_entry_count, 16);
    log_number_u("elf: PH Entry Size", header.ph_entry_size, 16);
    log_number_u("elf: SH Table", header.sh_offset, 16);
    log_number_u("elf: SH Entries", header.sh_entry_count, 16);
    log_number_u("elf: SH Entry Size", header.sh_entry_size, 16);

    ElfProgramHeader ph;
    for (int ph_off = header.ph_offset; ph_off < header.ph_entry_count*header.ph_entry_size; ph_off += header.ph_entry_size) {
        elf_parse_program_header(buffer, &ph, ph_off);
        log_info("elf: Parsed Program Header.");
        log_number_u("elf: PH Type", ph.type, 16);
        log_number_u("elf: PH Offset", ph.offset, 16);
        log_number_u("elf: PH Virtual Address", ph.v_addr, 16);
        log_number_u("elf: PH Physical Address", ph.p_addr, 16);
        log_number_u("elf: PH File Size", ph.file_size, 16);
        log_number_u("elf: PH Memory Size", ph.mem_size, 16);
        log_number_u("elf: PH Flags", ph.flags, 16);
        log_number_u("elf: PH Align", ph.align, 16);
    }

    free(buffer);
    close(fd);
}