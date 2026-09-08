#include "greet.h"
#include "sched.h"      /* task_t, g_tasks, sched_alloc_task, etc. */
#include "elf.h"        /* elf_load, elf_load_result_t */
#include "fat32.h"      /* fat32_read_file */
#include "mm.h"         /* kmalloc/kfree */

#define STACK_SIZE 8192

extern task_t *current_task; /* whatever your scheduler exposes today */

static uint8_t g_elf_load_buf[65536]; /* same pattern as sys_exec */

int greet_fork(void) {
    task_t *parent = current_task;
    task_t *child  = sched_alloc_task();
    if (!child) return -1;

    void *new_stack = kmalloc(STACK_SIZE);
    if (!new_stack) return -1;

    /* copy parent's stack contents byte-for-byte so the child
       resumes at the exact same point parent was at */
    for (uint32_t i = 0; i < STACK_SIZE; i++) {
        ((uint8_t *)new_stack)[i] = ((uint8_t *)parent->stack_base)[i];
    }

    uint32_t offset = parent->esp - (uint32_t)parent->stack_base;
    child->stack_base = new_stack;
    child->esp        = (uint32_t)new_stack + offset;
    child->priority    = parent->priority;
    child->state        = TASK_READY;

    /* child's return value is 0 (fork convention); parent's is
       child->id - set on whichever return path your scheduler
       uses when it resumes a task */
    return child->id;
}

int greet_exec(const char *path) {
    uint32_t size;
    if (fat32_read_file(path, g_elf_load_buf, sizeof(g_elf_load_buf), &size) != 0) {
        return -1;
    }

    elf_load_result_t result;
    if (elf_load(g_elf_load_buf, size, &result) != 0) {
        return -1;
    }

    /* jumps into the new entry point on the SAME task/stack -
       does not return on success */
    __asm__ volatile ("jmp *%0" : : "r"(result.entry));
    return -1; /* unreachable unless elf_load failed to set entry */
}

int greet_spawn(const char *path) {
    uint32_t size;
    if (fat32_read_file(path, g_elf_load_buf, sizeof(g_elf_load_buf), &size) != 0) {
        return -1;
    }

    elf_load_result_t result;
    if (elf_load(g_elf_load_buf, size, &result) != 0) {
        return -1;
    }

    task_t *child = sched_alloc_task();
    if (!child) return -1;

    void *stack = kmalloc(STACK_SIZE);
    if (!stack) return -1;

    child->stack_base = stack;
    child->esp        = (uint32_t)stack + STACK_SIZE;
    /* set child's saved eip to result.entry the same way your
       scheduler's task_create sets it up for a fresh task */
    sched_set_entry(child, result.entry);
    child->state = TASK_READY;

    return child->id;
}