#include <stddef.h>
#include "mm_private.h"

void mem_init(uint32_t total_ram);
void *kmalloc(uint32_t size);
void kfree(void *ptr);
void *krealloc(void *ptr, uint32_t new_size);

#define HEAP_CAPACITY (64u * 1024u)
#define MIN_SPLIT_SIZE (sizeof(block_t) + 16u)

static unsigned char g_heap[HEAP_CAPACITY];
static block_t *g_first_block = 0;

void mem_init(uint32_t total_ram)
{
    size_t capacity = HEAP_CAPACITY;

    if (total_ram != 0 && total_ram < capacity) {
        capacity = total_ram;
    }

    if (capacity < sizeof(block_t)) {
        capacity = sizeof(block_t);
    }

    g_first_block = (block_t *)g_heap;
    g_first_block->size = capacity - sizeof(block_t);
    g_first_block->used = 0;
    g_first_block->next = 0;
}

void *kmalloc(uint32_t size)
{
    block_t *current;
    block_t *split;
    size_t request;
    size_t leftover;

    if (size == 0) {
        return 0;
    }

    request = mm_align_size(size);
    current = g_first_block;

    while (current != 0) {
        if (!current->used && current->size >= request) {
            leftover = current->size - request;

            if (leftover >= MIN_SPLIT_SIZE) {
                split = (block_t *)((unsigned char *)current + sizeof(block_t) + request);
                split->size = leftover - sizeof(block_t);
                split->used = 0;
                split->next = current->next;

                current->size = request;
                current->used = 1;
                current->next = split;
            } else {
                current->used = 1;
            }

            return mm_block_payload(current);
        }

        current = current->next;
    }

    return 0;
}

void kfree(void *ptr)
{
    block_t *block;
    block_t *prev;

    if (ptr == 0 || g_first_block == 0) {
        return;
    }

    block = (block_t *)((unsigned char *)ptr - sizeof(block_t));
    block->used = 0;

    /* coalesce with next first, so a later prev-merge absorbs everything in one pass */
    if (block->next != 0 && block->next->used == 0) {
        block->size += block->next->size + sizeof(block_t);
        block->next = block->next->next;
    }

    prev = mm_find_previous_block(g_first_block, block);
    if (prev != 0 && prev->used == 0) {
        prev->size += block->size + sizeof(block_t);
        prev->next = block->next;
    }
}

/*
 * krealloc: grow/shrink an existing allocation.
 *
 * - ptr == 0            -> behaves like kmalloc(new_size)
 * - new_size == 0        -> behaves like kfree(ptr), returns 0
 * - shrinking            -> in-place, splits off the remainder as a free block
 *                           if there's enough room to make a useful split
 * - growing, next block
 *   is free and big enough -> absorbs the next block in place (no copy)
 * - otherwise             -> allocate new, copy min(old,new) bytes, free old
 */
void *krealloc(void *ptr, uint32_t new_size)
{
    block_t *block;
    block_t *next;
    size_t request;
    size_t old_size;
    size_t combined;
    size_t leftover;
    void *newptr;
    unsigned char *src;
    unsigned char *dst;
    size_t copy_size;
    size_t i;

    if (ptr == 0) {
        return kmalloc(new_size);
    }

    if (new_size == 0) {
        kfree(ptr);
        return 0;
    }

    block = (block_t *)((unsigned char *)ptr - sizeof(block_t));
    old_size = block->size;
    request = mm_align_size(new_size);

    /* already big enough - shrink in place if the remainder is worth splitting */
    if (request <= old_size) {
        leftover = old_size - request;

        if (leftover >= MIN_SPLIT_SIZE) {
            block_t *split = (block_t *)((unsigned char *)block + sizeof(block_t) + request);
            split->size = leftover - sizeof(block_t);
            split->used = 0;
            split->next = block->next;

            block->size = request;
            block->next = split;

            /* merge the new remainder forward if the block after it is also free */
            if (split->next != 0 && split->next->used == 0) {
                split->size += split->next->size + sizeof(block_t);
                split->next = split->next->next;
            }
        }

        return ptr;
    }

    /* growing: try to absorb the next block in place if it's free and big enough */
    next = block->next;
    if (next != 0 && next->used == 0) {
        combined = old_size + sizeof(block_t) + next->size;

        if (combined >= request) {
            leftover = combined - request;

            if (leftover >= MIN_SPLIT_SIZE) {
                block_t *split = (block_t *)((unsigned char *)block + sizeof(block_t) + request);
                split->size = leftover - sizeof(block_t);
                split->used = 0;
                split->next = next->next;

                block->size = request;
                block->next = split;
            } else {
                block->size = combined;
                block->next = next->next;
            }

            return ptr;
        }
    }

    /* no room in place - allocate new, copy, free old */
    newptr = kmalloc(new_size);
    if (newptr == 0) {
        return 0;
    }

    copy_size = (old_size < new_size) ? old_size : new_size;
    src = (unsigned char *)ptr;
    dst = (unsigned char *)newptr;
    for (i = 0; i < copy_size; i++) {
        dst[i] = src[i];
    }

    kfree(ptr);
    return newptr;
}
