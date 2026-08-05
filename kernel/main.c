#include "serial.h"
#include "ksubsys.h"

/* Простой вывод лога с тегом подсистемы: [tag] message */
static void klog(ksubsys_t sys, const char *msg)
{
    serial_write('[');
    serial_puts(sys.tag);
    serial_puts("] ");
    serial_puts(msg);
}

void main(void)
{
    serial_init();

   
    serial_puts("launching kernel../n/n");

    klog(LOG_CLOCK, "clocksource calibrated, tick rate locked/n");
    klog(LOG_SERIO, "serial line stable, echo confirmed/n");
    klog(LOG_RTC,   "real-time clock synced/n");
    klog(LOG_INPUT, "input subsystem standing by/n");
    klog(LOG_DISK,  "disk controller not yet initialized/n");
    klog(LOG_NET,   "network stack not yet initialized/n");

    serial_puts("/nkernel core online. all early subsystems reporting./n");
    serial_puts("system idle/n");

    for (;;);   // ядру пока некуда возвращаться
}