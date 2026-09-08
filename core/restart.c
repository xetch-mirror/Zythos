#include <stdint.h>

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

/* Reboots via the 8042 keyboard controller reset line.
   Works on essentially all real x86 hardware. */
void restart(void) {
    uint8_t status;

    do {
        status = inb(0x64);
    } while (status & 0x02);   /* wait for input buffer to be empty */

    outb(0x64, 0xFE);          /* pulse CPU reset line */

    __asm__ volatile ("cli");
    for (;;) {
        __asm__ volatile ("hlt");
    }
}