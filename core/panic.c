/*
 * panic.c — реализация паники ядра
 */

#include "panic.h"
#include "serial.h"
#include "ksubsys.h"

DEFINE_SUBSYS(LOG_PANIC, "panic");

/* Таблица человекочитаемых причин — держим в одном месте */
static const char *panic_cause_str(panic_cause_t cause)
{
    switch (cause) {
        case PANIC_GENERAL_PROTECTION_FAULT:
            return "General Protection Fault";
        case PANIC_INIT_DIED:
            return "init died";
        case PANIC_VFS_NO_BLOCKDEV:
            return "VFS: could not find blockdev 0,0";
        case PANIC_ROOTFS_NOT_MOUNTED:
            return "rootfs not mounted";
        case PANIC_INIT_MISSING:
            return "init missing";
        case PANIC_CANNOT_EXEC_INIT:
            return "kernel could not run init";
        case PANIC_KERNEL_DIED:
            return "kernel died";
        default:
            return "unknown panic";
    }
}

/* Простой freestanding-вывод беззнакового hex, без libc */
static void serial_puts_hex(uint32_t val)
{
    static const char digits[] = "0123456789abcdef";
    char buf[11]; /* "0x" + 8 hex + '\0' */
    int i;

    buf[0] = '0';
    buf[1] = 'x';
    for (i = 0; i < 8; i++) {
        buf[9 - i] = digits[val & 0xF];
        val >>= 4;
    }
    buf[10] = '\0';
    serial_puts(buf);
}

static void dump_regs(panic_regs_t *r)
{
    if (!r)
        return;

    serial_puts("eax="); serial_puts_hex(r->eax);
    serial_puts(" ebx="); serial_puts_hex(r->ebx);
    serial_puts(" ecx="); serial_puts_hex(r->ecx);
    serial_puts(" edx="); serial_puts_hex(r->edx);
    serial_puts("/n");

    serial_puts("esi="); serial_puts_hex(r->esi);
    serial_puts(" edi="); serial_puts_hex(r->edi);
    serial_puts(" ebp="); serial_puts_hex(r->ebp);
    serial_puts(" esp="); serial_puts_hex(r->esp);
    serial_puts("/n");

    serial_puts("eip="); serial_puts_hex(r->eip);
    serial_puts(" err="); serial_puts_hex(r->err_code);
    serial_puts("/n");
}

__attribute__((noreturn))
void panic(panic_cause_t cause, const char *extra, panic_regs_t *regs)
{
    /* Отключаем прерывания — паника не должна прерываться сама собой */
    __asm__ volatile ("cli");

    serial_puts("/n*** KERNEL PANIC [");
    serial_puts(LOG_PANIC.tag);
    serial_puts("] ***/n");

    serial_puts("cause: ");
    serial_puts(panic_cause_str(cause));
    serial_puts("/n");

    if (extra) {
        serial_puts("info: ");
        serial_puts(extra);
        serial_puts("/n");
    }

    dump_regs(regs);

    serial_puts("system halted./n");

    /* Останавливаемся навсегда */
    for (;;) {
        __asm__ volatile ("hlt");
    }
}