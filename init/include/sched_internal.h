#ifndef SCHED_INTERNAL_H
#define SCHED_INTERNAL_H

#include "sys_types.h"

typedef struct task {
    task_id_t id;
    task_state_t state;
    uint32_t priority;
    uint32_t esp;            /* сохранённый указатель стека при переключении */
    uint8_t *stack_base;     /* выделено через kmalloc, для освобождения при exit */
    uint32_t wake_tick;      /* для sys_sleep — не реальное время, см. TODO ниже */
    void (*entry)(void);
    struct task *next;
} task_t;

extern task_t *g_current_task;

#endif