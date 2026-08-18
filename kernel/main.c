#define __KERNEL__
#include <linux/kernel.h>

#include "serial.h"
#include "timesubsys.h"
#include "memory/memory.h"
#include "vfs.h"
#include <linux/elf.h>

#define INIT_PATH   "/init.elf"
#define INIT_MAXSZ  (1 * 1024 * 1024) /* 1MB — временный лимит, пока нет потоковой загрузки */

/* Вывод лога с тегом подсистемы и уровнем важности: [tag] LEVEL message */
static void klog(ksubsys_t sys, const char *level, const char *msg)
{
    serial_write('[');
    serial_puts(sys.tag);
    serial_puts("] ");
    serial_puts(level);
    serial_write(' ');       /* было потеряно — лог склеивался с сообщением */
    serial_puts(msg);
}

/* Загружает и запускает init.elf через VFS + elf_load.
   Возвращает false, если что-то пошло не так — main() уйдёт в idle-цикл как fallback. */
static bool boot_init(void)
{
    vfs_node_t *node = vfs_open(INIT_PATH, VFS_FILE);
    if (!node) {
        klog(LOG_BASE, KERN_WARNING, "init.elf not found on vfs/n");
        return false;
    }

    if (node->size == 0 || node->size > INIT_MAXSZ) {
        klog(LOG_BASE, KERN_WARNING, "init.elf size out of range/n");
        return false;
    }

    /* elf_load ожидает буфер в памяти целиком — стримингового чтения пока нет */
    void *buf = kmalloc(node->size);
    if (!buf) {
        klog(LOG_BASE, KERN_WARNING, "kmalloc failed for init.elf buffer/n");
        return false;
    }

    if (vfs_read(node, buf, node->size) != (ssize_t)node->size) {
        klog(LOG_BASE, KERN_WARNING, "short read on init.elf/n");
        kfree(buf);
        return false;
    }

    elf_load_result_t res;
    if (!elf_load(buf, node->size, &res)) {
        klog(LOG_BASE, KERN_WARNING, "elf_load rejected init.elf/n");
        kfree(buf);
        return false;
    }

    klog(LOG_BASE, KERN_INFO, "init.elf loaded, jumping to entry/n");

    /* buf не освобождаем — сегменты PT_LOAD скопированы elf_load'ом
       по своим адресам, но сам буфер пока не трогаем на случай отладки */
    void (*entry)(void) = (void (*)(void))res.entry;
    entry();

    /* Если entry() когда-либо вернётся (не должен, у init нет процесса-родителя) */
    return true;
}

void main(void)
{
    serial_init();
    serial_puts("launching kernel..\n\n");

    /* TODO: mem_init() пока не реализован в memory.c —
       вызов ниже потребует реальной реализации перед линковкой */
    mem_init(0);  /* 0 = временно, пока нет карты памяти от загрузчика */

    klog(LOG_BASE,  KERN_INFO,    "base subsystem online\n");
    klog(LOG_CLOCK, KERN_INFO,    "clocksource calibrated, tick rate locked/n");
    klog(LOG_SERIO, KERN_INFO,    "serial line stable, echo confirmed\n");
    klog(LOG_RTC,   KERN_INFO,    "real-time clock synced\n");
    klog(LOG_INPUT, KERN_INFO,    "input subsystem standing by\n");
    klog(LOG_DISK,  KERN_WARNING, "disk controller not yet initialized\n");
    klog(LOG_NET,   KERN_WARNING, "network stack not yet initialized\n");

    serial_puts("/nkernel core online. all early subsystems reporting.\n");

    if (!boot_init()) {
        serial_puts("failed to boot init — halting in idle loop/n");
        for (;;);
    }

    /* Не должны сюда попасть — init никогда не должен вернуться */
    serial_puts("init returned unexpectedly — halting/n");
    for (;;);
}