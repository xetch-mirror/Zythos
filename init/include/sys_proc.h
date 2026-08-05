#ifndef SYS_PROC_H
#define SYS_PROC_H

#include "sys_io.h"
#include "sys_call.h"

static inline void console_subsystem(void) {
    io_print("[CON] environment online\n");
    while (1) {
        sys_sleep(500);
    }
}

#endif