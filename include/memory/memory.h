#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

/* пока не реализовано */
void  mem_init(uint32_t total_ram);
void *kmalloc(uint32_t size);
void  kfree(void *ptr);

#endif // MEMORY_H