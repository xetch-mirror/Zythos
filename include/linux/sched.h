#ifndef _LINUX_SCHED_H
#define _LINUX_SCHED_H

#include <linux/types.h>

#define TASK_SIZE       0xC0000000UL

extern int wp_works_ok;

struct task_struct {
        int euid;
        char name[64];
        struct task_struct *next;
        struct task_struct *next_sibling;
};

extern struct task_struct *current;

#endif