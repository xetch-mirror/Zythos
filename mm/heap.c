#include <stddef.h>
#include "mm_private.h"

void mem_init(uint32_t total_ram);
void *kmalloc(uint32_t size);
void kfree(void *ptr);

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
    block_t *prev;
    block_t *split;
    size_t request;
    size_t leftover;

    if (size == 0) {
        return 0;
    }

    request = mm_align_size(size);
    current = g_first_block;
    prev = 0;

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

        prev = current;
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

    prev = mm_find_previous_block(g_first_block, block);
    if (prev != 0 && prev->used == 0) {
        prev->size += block->size + sizeof(block_t);
        prev->next = block->next;
        block = prev;
    }

    if (block->next != 0 && block->next->used == 0) {
        block->size += block->next->size + sizeof(block_t);
        block->next = block->next->next;
    }
}
