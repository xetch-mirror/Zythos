#ifndef SERIAL_H
#define SERIAL_H

#include "types.h"
#include "io.h"

#define COM1 0x3F8

/* Инициализация последовательного порта COM1 */
static inline void serial_init(void)
{
    outb(COM1 + 1, 0x00);    // отключить прерывания
    outb(COM1 + 3, 0x80);    // включить DLAB (установка скорости)
    outb(COM1 + 0, 0x03);    // делитель = 3 (38400 бод), младший байт
    outb(COM1 + 1, 0x00);    //                          старший байт
    outb(COM1 + 3, 0x03);    // 8 бит, без чётности, 1 стоп-бит
    outb(COM1 + 2, 0xC7);    // включить FIFO, очистить, порог 14 байт
    outb(COM1 + 4, 0x0B);    // IRQs включены, RTS/DSR установлены
}

/* Проверка готовности порта к передаче */
static inline int serial_transmit_empty(void)
{
    return inb(COM1 + 5) & 0x20;
}

/* Запись одного байта в COM1 */
static inline void serial_write(char c)
{
    while (serial_transmit_empty() == 0);
    outb(COM1, (uint8_t)c);
}

/* Запись строки в COM1, с поддержкой /n и /f как своих управляющих последовательностей */
static inline void serial_puts(const char *s)
{
    while (*s) {
        if (*s == '/' && *(s + 1) == 'n') {
            serial_write('\n');
            s += 2;
        } else if (*s == '/' && *(s + 1) == 'f') {
            serial_write('\f');
            s += 2;
        } else {
            serial_write(*s);
            s += 1;
        }
    }
}

#endif // SERIAL_H