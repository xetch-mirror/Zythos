#include "elf.h"
#include "../kernel/include/timesubsys.h"

static void klog(ksubsys_t sys, const char *format, ...)
{
    (void)sys;
    (void)format;
}

static void *loader_memcpy(void *destination, const void *source, uint32_t size)
{
    uint8_t *dst = (uint8_t *)destination;
    const uint8_t *src = (const uint8_t *)source;

    for (uint32_t index = 0; index < size; index++) {
        dst[index] = src[index];
    }

    return destination;
}

static void *loader_memset(void *destination, int value, uint32_t size)
{
    uint8_t *dst = (uint8_t *)destination;

    for (uint32_t index = 0; index < size; index++) {
        dst[index] = (uint8_t)value;
    }

    return destination;
}

/*
 * Загрузчик ELF32 для Zythos.
 * Ожидает, что весь файл образа уже находится в памяти (buf),
 * например после чтения с fs через fat32/ext2 драйвер.
 *
 * Возвращает точку входа (entry point) через out_entry.
 * Возвращает 0 при успехе, -1 при ошибке.
 */

static int elf_check_header(const Elf32_Ehdr *eh)
{
    if (eh->e_ident[EI_MAG0] != ELFMAG0 ||
        eh->e_ident[EI_MAG1] != ELFMAG1 ||
        eh->e_ident[EI_MAG2] != ELFMAG2 ||
        eh->e_ident[EI_MAG3] != ELFMAG3) {
        klog(LOG_LOADER, "elf: bad magic\n");
        return -1;
    }

    if (eh->e_ident[EI_CLASS] != ELFCLASS32) {
        klog(LOG_LOADER, "elf: not 32-bit\n");
        return -1;
    }

    if (eh->e_ident[EI_DATA] != ELFDATA2LSB) {
        klog(LOG_LOADER, "elf: not little-endian\n");
        return -1;
    }

    if (eh->e_machine != EM_386) {
        klog(LOG_LOADER, "elf: wrong machine (want EM_386)\n");
        return -1;
    }

    if (eh->e_type != ET_EXEC) {
        /* ET_DYN (PIE) would need relocation handling — not supported yet */
        klog(LOG_LOADER, "elf: only ET_EXEC supported for now\n");
        return -1;
    }

    if (eh->e_phoff == 0 || eh->e_phnum == 0) {
        klog(LOG_LOADER, "elf: no program headers\n");
        return -1;
    }

    return 0;
}

/*
 * buf       — начало ELF-образа в памяти (весь файл)
 * buf_size  — размер буфера, для проверки границ при чтении заголовков
 * out       — результат: entry point и границы занятой памяти
 *
 * ВНИМАНИЕ: эта версия копирует сегменты по их физическим/виртуальным
 * адресам напрямую (identity-mapped kernel space). Как только появится
 * настоящий VMM с page tables per-process, здесь нужно будет сначала
 * выделять и мапить страницы под p_vaddr, а потом копировать.
 */
int elf_load(const void *buf, uint32_t buf_size, elf_load_result_t *out)
{
    const Elf32_Ehdr *eh = (const Elf32_Ehdr *)buf;

    if (buf_size < sizeof(Elf32_Ehdr)) {
        klog(LOG_LOADER, "elf: buffer too small for header\n");
        return -1;
    }

    if (elf_check_header(eh) != 0)
        return -1;

    if (eh->e_phoff + (uint32_t)eh->e_phnum * eh->e_phentsize > buf_size) {
        klog(LOG_LOADER, "elf: program header table out of bounds\n");
        return -1;
    }

    const Elf32_Phdr *phdrs =
        (const Elf32_Phdr *)((const uint8_t *)buf + eh->e_phoff);

    Elf32_Addr load_min = 0xFFFFFFFF;
    Elf32_Addr load_max = 0;
    int loaded_any = 0;

    for (Elf32_Half i = 0; i < eh->e_phnum; i++) {
        const Elf32_Phdr *ph = &phdrs[i];

        if (ph->p_type != PT_LOAD)
            continue; /* пропускаем PT_DYNAMIC/PT_NOTE/PT_INTERP и т.д. */

        if (ph->p_offset + ph->p_filesz > buf_size) {
            klog(LOG_LOADER, "elf: segment %d out of file bounds\n", i);
            return -1;
        }

        if (ph->p_filesz > ph->p_memsz) {
            klog(LOG_LOADER, "elf: segment %d filesz > memsz\n", i);
            return -1;
        }

        /* TODO: once VMM exists — allocate physical frames and map
         * ph->p_vaddr .. ph->p_vaddr+p_memsz with permissions derived
         * from ph->p_flags (PF_R/PF_W/PF_X) before writing anything. */

        void *dst = (void *)(uintptr_t)ph->p_vaddr;
        const void *src = (const uint8_t *)buf + ph->p_offset;

        /* копируем содержимое файла */
        loader_memcpy(dst, src, ph->p_filesz);

        /* обнуляем .bss-хвост (memsz - filesz) */
        if (ph->p_memsz > ph->p_filesz) {
                 loader_memset((uint8_t *)dst + ph->p_filesz, 0,
                         ph->p_memsz - ph->p_filesz);
        }

        Elf32_Addr seg_start = ph->p_vaddr;
        Elf32_Addr seg_end   = ph->p_vaddr + ph->p_memsz;

        if (seg_start < load_min) load_min = seg_start;
        if (seg_end   > load_max) load_max = seg_end;

        loaded_any = 1;

        klog(LOG_LOADER, "elf: loaded segment %d vaddr=%x filesz=%x memsz=%x flags=%x\n",
             i, ph->p_vaddr, ph->p_filesz, ph->p_memsz, ph->p_flags);
    }

    if (!loaded_any) {
        klog(LOG_LOADER, "elf: no PT_LOAD segments found\n");
        return -1;
    }

    out->entry = eh->e_entry;
    out->load_min = load_min;
    out->load_max = load_max;

    klog(LOG_LOADER, "elf: entry=%x range=[%x-%x]\n",
         eh->e_entry, load_min, load_max);

    return 0;
}

/*
 * Пример использования из кода ядра, после чтения файла с диска в буфер:
 *
 *   elf_load_result_t res;
 *   if (elf_load(file_buf, file_size, &res) == 0) {
 *       void (*entry)(void) = (void (*)(void))res.entry;
 *       entry();   // или переключение контекста, если это процесс
 *   }
 */
