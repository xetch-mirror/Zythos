#ifndef MM_PRIVATE_H
#define MM_PRIVATE_H

#include <stddef.h>

typedef unsigned int uint32_t;

typedef struct block {
    size_t size;
    unsigned char used;
    struct block *next;
} block_t;

size_t mm_align_size(size_t size);
block_t *mm_find_previous_block(block_t *first_block, block_t *block);
void *mm_block_payload(block_t *block);

#endif
