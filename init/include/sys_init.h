#ifndef SYS_INIT_H
#define SYS_INIT_H

#include "sys_types.h"

typedef enum {
    BOOT_STAGE_EARLY = 0,
    BOOT_STAGE_DRIVERS,
    BOOT_STAGE_HEADERS,
    BOOT_STAGE_INIT,
    BOOT_STAGE_RUNNING
} boot_stage_t;

void sys_set_boot_stage(boot_stage_t stage);
boot_stage_t sys_get_boot_stage(void);
int sys_launch_init(void (*init_entry)(void));

#endif // SYS_INIT_H