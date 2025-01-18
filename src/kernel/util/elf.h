#ifndef ELF_H
#define ELF_H

#include "core/fs/fs.h"

#define ELF_CLASS_32 1
#define ELF_CLASS_64 2

#define ELF_ENDIANNESS_LITTLE 1
#define ELF_ENDIANNESS_BIG 2

#define ELF_TYPE_UNKNOWN 1
#define ELF_TYPE_RELOCATABLE 1
#define ELF_TYPE_EXECUTABLE 2
#define ELF_TYPE_SHARED 3
#define ELF_TYPE_CORE 4

#define ELF_MACHINE_X86 3

typedef struct {
    uint8_t magic[4];
    uint8_t class;
    uint8_t endianness;
    uint8_t version;
    uint8_t os_abi;
    uint8_t abi_version;
    uint8_t pad[7];
} __attribute__((packed)) ElfIdent;

typedef struct {
    ElfIdent ident;        // ELF identification
    uint16_t type;          // Object file type
    uint16_t machine;       // Architecture
    uint32_t version;       // Object file version
    uint32_t entry;         // Entry point virtual address
    uint32_t ph_offset;     // Program header table file offset
    uint32_t sh_offset;     // Section header table file offset
    uint32_t flags;         // Processor-specific flags
    uint16_t header_size;   // ELF header size in bytes
    uint16_t ph_entry_size; // Program header table entry size
    uint16_t ph_entry_count;// Program header table entry count
    uint16_t sh_entry_size; // Section header table entry size
    uint16_t sh_entry_count;// Section header table entry count
    uint16_t sh_str_index;  // Section header string table index
} __attribute__((packed)) ElfHeader;

#define ELF_PT_LOAD 1
#define ELF_PT_DYNAMIC 2

typedef struct {
    uint32_t type;       // Segment type
    uint32_t offset;     // Segment file offset
    uint32_t v_addr;     // Segment virtual address
    uint32_t p_addr;     // Segment physical address
    uint32_t file_size;  // Segment size in file
    uint32_t mem_size;   // Segment size in memory
    uint32_t flags;      // Segment flags
    uint32_t align;      // Segment alignment
} __attribute__((packed)) ElfProgramHeader;

int elf_parse_header(uint8_t* buffer, ElfHeader* header);
int elf_parse_program_header(uint8_t* buffer, ElfProgramHeader* header, uint32_t ph_offset);


#endif