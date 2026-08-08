#ifndef _ELF_H
#define _ELF_H

#include <stdint.h>

/* ELF32 — заголовки и структуры для загрузчика Zythos */

typedef uint32_t Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef int32_t  Elf32_Sword;
typedef uint32_t Elf32_Word;

#define EI_NIDENT 16

/* e_ident indices */
#define EI_MAG0       0
#define EI_MAG1       1
#define EI_MAG2       2
#define EI_MAG3       3
#define EI_CLASS      4
#define EI_DATA       5
#define EI_VERSION    6
#define EI_OSABI      7

#define ELFMAG0 0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

#define ELFCLASS32 1
#define ELFDATA2LSB 1

/* e_type */
#define ET_NONE 0
#define ET_REL  1
#define ET_EXEC 2
#define ET_DYN  3

/* e_machine */
#define EM_386 3

typedef struct {
    unsigned char e_ident[EI_NIDENT];
    Elf32_Half    e_type;
    Elf32_Half    e_machine;
    Elf32_Word    e_version;
    Elf32_Addr    e_entry;      /* точка входа */
    Elf32_Off     e_phoff;      /* смещение program header table */
    Elf32_Off     e_shoff;      /* смещение section header table */
    Elf32_Word    e_flags;
    Elf32_Half    e_ehsize;
    Elf32_Half    e_phentsize;
    Elf32_Half    e_phnum;
    Elf32_Half    e_shentsize;
    Elf32_Half    e_shnum;
    Elf32_Half    e_shstrndx;
} Elf32_Ehdr;

/* p_type */
#define PT_NULL    0
#define PT_LOAD    1
#define PT_DYNAMIC 2
#define PT_INTERP  3
#define PT_NOTE    4
#define PT_BSS     8 /* not standard, some loaders alias this - ignore */

/* p_flags */
#define PF_X 0x1
#define PF_W 0x2
#define PF_R 0x4

typedef struct {
    Elf32_Word p_type;
    Elf32_Off  p_offset;   /* смещение сегмента в файле */
    Elf32_Addr p_vaddr;    /* виртуальный адрес назначения */
    Elf32_Addr p_paddr;    /* физический адрес (обычно = vaddr) */
    Elf32_Word p_filesz;   /* размер в файле */
    Elf32_Word p_memsz;    /* размер в памяти (>= filesz, остаток = .bss) */
    Elf32_Word p_flags;
    Elf32_Word p_align;
} Elf32_Phdr;

/* Section headers — нужны только если потом захотим symtab/relocs */
typedef struct {
    Elf32_Word sh_name;
    Elf32_Word sh_type;
    Elf32_Word sh_flags;
    Elf32_Addr sh_addr;
    Elf32_Off  sh_offset;
    Elf32_Word sh_size;
    Elf32_Word sh_link;
    Elf32_Word sh_info;
    Elf32_Word sh_addralign;
    Elf32_Word sh_entsize;
} Elf32_Shdr;

#endif /* _ELF_H */
