#define __KERNEL__
#include <linux/kernel.h>

#include "serial.h"
#include "timesubsys.h"
#include "memory/memory.h"

/* Вывод лога с тегом подсистемы и уровнем важности: [tag] LEVEL message */
static void klog(ksubsys_t sys, const char *level, const char *msg)
{
    serial_write('[');
    serial_puts(sys.tag);
    serial_puts("] ");
    serial_puts(level);
    serial_puts(msg);
}

void main(void)
{
    serial_init();
    serial_puts("launching kernel../n/n");

    /* TODO: mem_init() пока не реализован в memory.c —
       вызов ниже потребует реальной реализации перед линковкой */
    mem_init(0);  /* 0 = временно, пока нет карты памяти от загрузчика */

    klog(LOG_BASE,  KERN_INFO,    "base subsystem online/n");
    klog(LOG_CLOCK, KERN_INFO,    "clocksource calibrated, tick rate locked/n");
    klog(LOG_SERIO, KERN_INFO,    "serial line stable, echo confirmed/n");
    klog(LOG_RTC,   KERN_INFO,    "real-time clock synced/n");
    klog(LOG_INPUT, KERN_INFO,    "input subsystem standing by/n");
    klog(LOG_DISK,  KERN_WARNING, "disk controller not yet initialized/n");
    klog(LOG_NET,   KERN_WARNING, "network stack not yet initialized/n");

    serial_puts("/nkernel core online. all early subsystems reporting./n");
    serial_puts("system idle/n");

    for (;;);
}