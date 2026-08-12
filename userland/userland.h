/* userspace.h - ring 3 entry point for jumping from kernel to a loaded ELF */

#ifndef USERSPACE_H
#define USERSPACE_H

#include <stdint.h>

/* Прыжок в userspace: настраивает стек и сегменты кольца 3, iret на entry */
void jump_to_userspace(uint32_t entry);

#endif