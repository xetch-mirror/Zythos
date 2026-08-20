#include "kbd_buffer.h"

static int32_t sys_read(int fd, void *buf, uint32_t count)
{
    (void)fd;
    char *out = (char *)buf;
    uint32_t i = 0;

    while (i < count) {
        char c;
        while (!kbd_buffer_pop(&c)) {
            sys_yield(); /* нет данных — уступаем CPU, ждём IRQ1 */
        }
        out[i++] = c;
        if (i >= 1)
            break; /* обычно read() для клавиш возвращает по одному символу */
    }

    return (int32_t)i;
}