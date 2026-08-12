/* userspace.c */

#include "userspace.h"

/* Селекторы GDT для user code/data — подставь свои реальные значения
   (обычно 0x1B и 0x23 при стандартном layout: null, kcode, kdata, ucode, udata) */
#define USER_CS 0x1B
#define USER_DS 0x23
#define USER_STACK_TOP 0x00800000  /* TODO: заменить на реально выделенный user stack */

void jump_to_userspace(uint32_t entry)
{
    __asm__ volatile (
        "cli\n"
        "mov $" "0x23" ", %%ax\n"   /* user data selector */
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"

        "push $0x23\n"              /* SS */
        "push %0\n"                 /* ESP */
        "pushf\n"                   /* EFLAGS */
        "push $0x1B\n"              /* CS */
        "push %1\n"                 /* EIP = entry */
        "iret\n"
        :
        : "r"((uint32_t)USER_STACK_TOP), "r"(entry)
        : "eax"
    );
}