#include "fs/fat32.h"
#include "drivers/elf.h"
#include "ksubsys.h"

#define ELF_LOAD_BUF_SIZE (256 * 1024)  /* adjust to available heap */
static uint8_t g_elf_load_buf[ELF_LOAD_BUF_SIZE];

static int sys_exec(const char *path)
{
    int size = fat32_read_file(path, g_elf_load_buf, ELF_LOAD_BUF_SIZE);
    if (size <= 0) {
        klog(LOG_LOADER, "exec: failed to read %s\n", path);
        return -1;
    }

    elf_load_result_t res;
    if (elf_load(g_elf_load_buf, (uint32_t)size, &res) != 0) {
        klog(LOG_LOADER, "exec: elf_load failed for %s\n", path);
        return -1;
    }

    klog(LOG_LOADER, "exec: jumping to entry=%x\n", res.entry);

    void (*entry)(void) = (void (*)(void))res.entry;
    entry();

    /* not reached under normal operation */
    return 0;
}

/* in your existing syscall dispatch switch, e.g. isr80 handler: */
case SYS_EXEC:
    return sys_exec((const char *)arg1);  /* arg1 = path pointer from userland */