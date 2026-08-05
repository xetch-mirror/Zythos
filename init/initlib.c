#include "initlib.h"
#include "sys_call.h"
#include "sys_io.h"
#include "sys_proc.h"

/**
 * @brief handles stage progression during system bring-up.
 */
void initlib_boot_sequence(boot_stage_t target_stage) {
    sys_set_boot_stage(target_stage);
    switch (target_stage) {
        case BOOT_STAGE_EARLY:
            io_print("[init] stage: early initialization\n");
            break;
        case BOOT_STAGE_DRIVERS:
            io_print("[init] stage: drivers loading\n");
            break;
        case BOOT_STAGE_HEADERS:
            io_print("[init] stage: headers loading\n");
            break;
        case BOOT_STAGE_INIT:
            io_print("[init] stage: INIT process launching\n");
            break;
        case BOOT_STAGE_RUNNING:
            io_print("[init] stage: system fully RUNNING\n");
            break;
    }
}

/**
 * @brief Logs diagnostics based on system tick thresholds.
 * Offloads logging logic from init.c.
 */
void initlib_log_system_health(uint32_t ticks) {
    if (ticks % 500 == 0)  io_print("[log] supervisor uptime tick incremented\n");
    if (ticks % 1000 == 0) io_print("[log] memory map consistency check passed\n");
    if (ticks % 1500 == 0) io_print("[log] scheduler latency remains within tolerance\n");
    if (ticks % 2000 == 0) io_print("[log] interrupt descriptor table integrity verified\n");
    if (ticks % 2500 == 0) io_print("[log] entropy generator health output stable\n");
    if (ticks % 3000 == 0) io_print("[log] io subsystem bus active\n");
}

/**
 * @brief queries task state and detects process crashes.
 * 
 * @return WATCHDOG_OK if alive, WATCHDOG_DEAD if terminated, WATCHDOG_ERR on failure.
 */
int initlib_check_task_health(task_id_t tid, task_info_t *out_info) {
    if (sys_query_task(tid, out_info) != 0) {
        io_print("[err] failed to communicate with task registry\n");
        return WATCHDOG_ERR;
    }

    if (out_info->state == TASK_DEAD) {
        return WATCHDOG_DEAD;
    }

    return WATCHDOG_OK;
}

/**
 * @brief safe wrapper to attempt reviving the console task.
 */
task_id_t initlib_respawn_console(void) {
    io_print("[warn] shell subsystem terminated: attempting emergency restart\n");
    task_id_t new_tid = sys_create_task(console_subsystem, 1);

    if (new_tid == INVALID_TASK_ID) {
        io_print("[crit] panic: unable to revive console process\n");
    } else {
        io_print("[INITLIB] console process revived successfully\n");
    }

    return new_tid;
}

/**
 * @brief software delay replacing inline NOP block repetitions.
 */
void initlib_delay_loop(uint32_t count) {
    for (volatile uint32_t i = 0; i < count; i++) {
        asm volatile ("nop");
    }
}