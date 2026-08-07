#include "mm_private.h"
#include "memory.h"
#include <stddef.h>

size_t mm_align_size(size_t size)
{
    if (size == 0) {
        return 8;
    }

    return (size + 7u) & ~7u;
}

block_t *mm_find_previous_block(block_t *first_block, block_t *block)
{
    block_t *current = first_block;

    while (current != 0 && current->next != 0 && current->next != block) {
        current = current->next;
    }

    if (current != 0 && current->next == block) {
        return current;
    }

    return 0;
}

void *mm_block_payload(block_t *block)
{
    return (void *)((unsigned char *)block + sizeof(block_t));
}
