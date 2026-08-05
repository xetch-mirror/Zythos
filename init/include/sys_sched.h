#ifndef SYS_SCHED_H
#define SYS_SCHED_H

#include "sys_types.h"

task_id_t sys_task_create(void (*entry_point)(void), uint32_t priority);
void sys_yield(void);
task_id_t sys_get_current_task_id(void);
int sys_get_task_info(task_id_t id, task_info_t *out_info);
void sys_task_exit(int code);

#endif // SYS_SCHED_H