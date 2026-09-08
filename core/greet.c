#ifndef GREET_H
#define GREET_H

#include <stdint.h>

/* Real fork(): clones current task into a new one, same code/data.
   Cheap because Zythos has no per-process address space yet -
   parent and child share the SAME globals/heap. This is fine for
   now but not real process isolation; note it for later once you
   have paging/VMM. */
int  greet_fork(void);

/* Real exec(): replaces the CALLING task's own image in place.
   Does not return on success - the task jumps into the new binary. */
int  greet_exec(const char *path);

/* spawn = fork+exec combined, done directly (skips duplicating the
   parent's stack just to overwrite it). This is what most callers
   actually want - "run this binary as a new task". */
int  greet_spawn(const char *path);

#endif