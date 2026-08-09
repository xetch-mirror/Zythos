#include "kbd_buffer.h"

static int32_t sys_read(int fd, void *buf, uint32_t count)
{
    (void)fd; /* пока не различаем stdin от других fd */
    char *out = (char *)buf;
    uint32_t i = 0;

    while (i < count) {
        char c;
        if (!kbd_buffer_pop(&c)) {
            break; /* нет больше данных прямо сейчас */
        }
        out[i++] = c;
    }

    return (int32_t)i;
}