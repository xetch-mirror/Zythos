#ifndef INITLIB_H
#define INITLIB_H

#include "sys_types.h"
#include "sys_init.h"

// watchdog timers
#define WATCHDOG_OK       0
#define WATCHDOG_DEAD    1
#define WATCHDOG_ERR    -1

// function declarations
void initlib_boot_sequence(boot_stage_t target_stage);
void initlib_log_system_health(uint32_t ticks);
int  initlib_check_task_health(task_id_t tid, task_info_t *out_info);
task_id_t initlib_respawn_console(void);
void initlib_delay_loop(uint32_t count);

#endif // INITLIB_H