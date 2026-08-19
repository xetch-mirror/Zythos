#include <syscalls.h>
#include "cmdtable.h"   /* REGISTER_CMD macro */

#define SYS_KILL 6   /* ASSUMPTION: next free syscall number after
                       * SYS_READ/SYS_WRITE/SYS_EXIT/SYS_EXEC=3.
                       * Confirm against your actual syscall.h numbering. */

static long sys_kill(long task_id) {
    return my_syscall1(SYS_KILL, task_id);
}

static long kill_atol(const char *s) {
    long result = 0;
    int neg = 0;

    if (*s == '-') { neg = 1; s++; }

    while (*s >= '0' && *s <= '9') {
        result = result * 10 + (*s - '0');
        s++;
    }

    return neg ? -result : result;
}

int cmd_kill(int argc, char **argv) {
    if (argc < 2) {
        sys_write_str("usage: kill <task_id>\n");
        return 1;
    }

    long task_id = kill_atol(argv[1]);
    if (task_id <= 0) {
        sys_write_str("kill: invalid task id\n");
        return 1;
    }

    long ret = sys_kill(task_id);
    if (ret < 0) {
        sys_write_str("kill: no such task\n");
        return 1;
    }

    return 0;
}

REGISTER_CMD("kill", cmd_kill);