// kernel/write_override.c
#include <sys/types.h>
#include "serial.h"

ssize_t write(int fd, const void *buf, size_t count)
{
    (void)fd; /* пока не различаем stdout/stderr, всё идёт в serial */
    const char *p = (const char *)buf;
    for (size_t i = 0; i < count; i++) {
        serial_write(p[i]);
    }
    return (ssize_t)count;
}