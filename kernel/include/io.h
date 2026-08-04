#ifndef _IO_H
#define _IO_H

#include "types.h"

#if defined(__i386__) || defined(__x86_64__)

/* Запись байта в порт ввода-вывода */
static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ __volatile__ ("outb %0, %1" : : "a"(val), "Nd"(port));
}

/* Чтение байта из порта ввода-вывода */
static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ __volatile__ ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* Запись слова (16 бит) в порт ввода-вывода */
static inline void outw(uint16_t port, uint16_t val)
{
    __asm__ __volatile__ ("outw %0, %1" : : "a"(val), "Nd"(port));
}

/* Чтение слова (16 бит) из порта ввода-вывода */
static inline uint16_t inw(uint16_t port)
{
    uint16_t ret;
    __asm__ __volatile__ ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/* Небольшая задержка для портов старого оборудования,
   требующих времени на стабилизацию */
static inline void io_wait(void)
{
    outb(0x80, 0);
}

#endif /* __i386__ || __x86_64__ */

#endif /* _IO_H */