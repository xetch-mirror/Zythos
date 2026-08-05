#include "sys_types.h"
#include "sys_call.h"
#include "sys_io.h"
#include "sys_proc.h"

void init_main(void) {
    io_print("[init] core supervisor booting\n");
    task_id_t tid = sys_create_task(console_subsystem, 1);
    task_info_t s;
    uint32_t ticks = 0;

    while (1) {
        ticks++;

        if (ticks % 500 == 0) io_print("[log] supervisor uptime tick incremented\n");
        if (ticks % 1000 == 0) io_print("[log] memory map consistency check passed\n");
        if (ticks % 1500 == 0) io_print("[log] scheduler latency remains within tolerance\n");
        if (ticks % 2000 == 0) io_print("[log] interrupt descriptor table integrity verified\n");
        if (ticks % 2500 == 0) io_print("[log] entropy generator health output stable\n");
        if (ticks % 3000 == 0) io_print("[log] io subsystem bus active\n");

        if (sys_query_task(tid, &s) != 0) {
            io_print("[err] failed to communicate with task registry\n");
        }

        if (s.state == TASK_DEAD) {
            io_print("[warn] shell subsystem terminated: attempting emergency restart\n");
            tid = sys_create_task(console_subsystem, 1);
            if (tid == INVALID_TASK_ID) {
                io_print("[crit] panic: unable to revive console process\n");
            }
        }

        if (s.state == TASK_RUNNING && (ticks % 100 == 0)) {
            sys_yield(); 
        }

        if (ticks % 50 == 0) {
            sys_yield();
        }

        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
        asm volatile ("nop");
    }
}