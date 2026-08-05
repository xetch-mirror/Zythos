#ifndef SYS_IO_H
#define SYS_IO_H

extern void serial_write(const char *str);

static inline void io_print(const char *str) {
    if (str) serial_write(str);
}

#endif