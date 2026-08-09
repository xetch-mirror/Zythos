/* Кооперативный планировщик с приоритетами.
   Переключение происходит ТОЛЬКО внутри sys_yield() — нет таймера/IRQ0,
   значит нет вытесняющей многозадачности. Задачи обязаны сами
   вызывать sys_yield(), иначе одна "жадная" задача заблокирует всех. */

#include "sched_internal.h"
#include "sys_call.h"
#include "memory.h"

#define TASK_STACK_SIZE 8192

extern void context_switch(uint32_t *old_esp_store, uint32_t new_esp);

task_t *g_current_task = 0;
static task_t *g_task_list = 0;
static task_id_t g_next_id = 1;
static uint32_t g_sched_ticks = 0; /* инкрементируется на каждом sys_yield —
                                       НЕ реальное время, без PIT это просто счётчик */

static void task_trampoline(void)
{
    g_current_task->entry();
    sys_task_exit(0); /* если entry когда-либо вернётся */
}

void sys_sched_init(void)
{
    /* Представляет ТЕКУЩИЙ поток выполнения (например, init_main,
       уже работающий на обычном стеке ядра) как задачу id=0 */
    task_t *boot = (task_t *)kmalloc(sizeof(task_t));
    boot->id = 0;
    boot->state = TASK_RUNNING;
    boot->priority = 0;
    boot->esp = 0;      /* заполнится при первом sys_yield() */
    boot->stack_base = 0;
    boot->wake_tick = 0;
    boot->entry = 0;
    boot->next = 0;

    g_task_list = boot;
    g_current_task = boot;
    g_next_id = 1;
}

task_id_t sys_create_task(void (*entry)(void), uint32_t prio)
{
    task_t *t = (task_t *)kmalloc(sizeof(task_t));
    if (!t) return INVALID_TASK_ID;

    t->stack_base = (uint8_t *)kmalloc(TASK_STACK_SIZE);
    if (!t->stack_base) {
        kfree(t);
        return INVALID_TASK_ID;
    }

    /* Строим фальшивый стек-кадр, который context_switch() "развернёт"
       через pop/pop/pop/pop/ret прямо в task_trampoline */
    uint32_t *sp = (uint32_t *)(t->stack_base + TASK_STACK_SIZE);
    sp -= 5;
    sp[0] = 0; /* edi */
    sp[1] = 0; /* esi */
    sp[2] = 0; /* ebx */
    sp[3] = 0; /* ebp */
    sp[4] = (uint32_t)task_trampoline; /* "адрес возврата" */

    t->id = g_next_id++;
    t->state = TASK_READY;
    t->priority = prio;
    t->esp = (uint32_t)sp;
    t->wake_tick = 0;
    t->entry = entry;

    t->next = g_task_list;
    g_task_list = t;

    return t->id;
}

static task_t *find_task(task_id_t id)
{
    for (task_t *t = g_task_list; t; t = t->next) {
        if (t->id == id) return t;
    }
    return 0;
}

int sys_query_task(task_id_t tid, task_info_t *info)
{
    task_t *t = find_task(tid);
    if (!t || !info) return -1;

    info->id = t->id;
    info->state = t->state;
    info->priority = t->priority;
    return 0;
}

task_id_t sys_get_current_task_id(void)
{
    return g_current_task ? g_current_task->id : INVALID_TASK_ID;
}

/* Выбирает следующую READY-задачу с наивысшим приоритетом.
   При равенстве приоритетов — round-robin среди задач того же уровня. */
static task_t *pick_next(void)
{
    task_t *best = 0;

    for (task_t *t = g_task_list; t; t = t->next) {
        if (t->state == TASK_BLOCKED && t->wake_tick <= g_sched_ticks) {
            t->state = TASK_READY; /* "будим" по истечении псевдо-времени */
        }
        if (t->state != TASK_READY && t != g_current_task) continue;
        if (t == g_current_task) continue;

        if (t->state == TASK_READY) {
            if (!best || t->priority > best->priority) {
                best = t;
            }
        }
    }

    return best ? best : g_current_task; /* некого переключать — остаёмся здесь */
}

void sys_yield(void)
{
    g_sched_ticks++;

    task_t *prev = g_current_task;
    task_t *next = pick_next();

    if (next == prev) {
        return; /* нет других READY-задач */
    }

    if (prev->state == TASK_RUNNING) {
        prev->state = TASK_READY;
    }
    next->state = TASK_RUNNING;
    g_current_task = next;

    context_switch(&prev->esp, next->esp);
}

void sys_sleep(uint32_t ms)
{
    /* TODO: нет PIT/таймера — "ms" здесь на самом деле означает
       количество вызовов sys_yield() другими задачами, а не
       реальные миллисекунды. Будет неточно, пока не появится IRQ0. */
    g_current_task->wake_tick = g_sched_ticks + ms;
    g_current_task->state = TASK_BLOCKED;
    sys_yield();
}

void sys_task_exit(int code)
{
    (void)code;
    g_current_task->state = TASK_DEAD;
    /* TODO: стек (stack_base) не освобождается здесь — задача остаётся
       в списке как TASK_DEAD для watchdog'а (initlib_check_task_health),
       память освобождается только когда кто-то реально её respawn'ит
       и удаляет старую запись. Утечка, если никто не следит. */
    sys_yield();
    for (;;) { } /* недостижимо, если sys_yield работает правильно */
}