/* Простой кольцевой буфер для символов клавиатуры,
   заполняется в IRQ1, читается через sys_read() */
#include "kbd_buffer.h"

#define KBD_BUF_SIZE 128

static volatile char g_buf[KBD_BUF_SIZE];
static volatile uint32_t g_head = 0;
static volatile uint32_t g_tail = 0;

void kbd_buffer_push(char c)
{
    uint32_t next = (g_head + 1) % KBD_BUF_SIZE;
    if (next == g_tail) {
        return; /* буфер полон — символ отбрасывается */
    }
    g_buf[g_head] = c;
    g_head = next;
}

bool kbd_buffer_pop(char *out)
{
    if (g_tail == g_head) {
        return false; /* пусто */
    }
    *out = g_buf[g_tail];
    g_tail = (g_tail + 1) % KBD_BUF_SIZE;
    return true;
}
#endif