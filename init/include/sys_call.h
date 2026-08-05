#ifndef SYS_CALL_H
#define SYS_CALL_H

#include "sys_types.h"

extern task_id_t sys_create_task(void (*entry)(void), uint32_t prio);
extern int       sys_query_task(task_id_t tid, task_info_t *info);
extern void      sys_yield(void);
extern void      sys_sleep(uint32_t ms);

#endif