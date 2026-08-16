/*
 * panic.h — обработка неустранимых ошибок ядра
 */

#ifndef _PANIC_H
#define _PANIC_H

#include "types.h"

/* Причины паники ядра */
typedef enum {
    PANIC_GENERAL_PROTECTION_FAULT,
    PANIC_INIT_DIED,
    PANIC_VFS_NO_BLOCKDEV,
    PANIC_ROOTFS_NOT_MOUNTED,
    PANIC_INIT_MISSING,
    PANIC_CANNOT_EXEC_INIT,
    PANIC_KERNEL_DIED,
    PANIC_UNKNOWN
} panic_cause_t;

/* Необязательный контекст (регистры на момент паники) */
typedef struct {
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi, ebp, esp;
    uint32_t eip;
    uint32_t err_code;   /* код ошибки от CPU, если применимо, иначе 0 */
} panic_regs_t;

/*
 * panic() — печатает причину, необязательный доп. текст и регистры
 * через serial, затем останавливает систему навсегда (cli; hlt loop).
 * Не возвращает управление.
 */
__attribute__((noreturn))
void panic(panic_cause_t cause, const char *extra, panic_regs_t *regs);

/* Удобный макрос для мест без готового panic_regs_t */
#define PANIC(cause, extra) panic((cause), (extra), 0)

#endif /* _PANIC_H */