#include "include/sys_types.h"

#define MAX_TASKS 32
#define STACK_SIZE 4096
#define TASK_UNUSED TASK_DEAD

typedef struct {
    uint32_t stack[STACK_SIZE];
    uint32_t esp;
    task_state_t state;
} task_t;

task_t tasks[MAX_TASKS];
int current_task = 0; // 0
task_id_t sys_create_task(void (*entry)(void), uint32_t prio) {
    (void)prio;
    for (int i = 1; i < MAX_TASKS; i++) {
        if (tasks[i].state == TASK_UNUSED) {
            uint32_t *stack_top = (uint32_t*)&tasks[i].stack[STACK_SIZE];

            // don't like stacks
            *(--stack_top) = (uint32_t)entry;  // EIP (Entry point)
            *(--stack_top) = 0;                // EBP
            *(--stack_top) = 0;                // EDI
            *(--stack_top) = 0;                // ESI
            *(--stack_top) = 0;                // EDX
            *(--stack_top) = 0;                // ECX
            *(--stack_top) = 0;                // EBX
            *(--stack_top) = 0;                // EAX

            tasks[i].esp = (uint32_t)stack_top;
            tasks[i].state = TASK_RUNNING;
            return (task_id_t)i;
        }
    }
    return INVALID_TASK_ID;
}