#include <stdint.h>

static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

/* No ACPI parsing yet, so this can't power off real hardware.
   These ports cover common emulators (QEMU, Bochs, VirtualBox) -
   useful since you're testing on v86. On real hardware this
   just falls through to the halt loop. */
void shutdown(void) {
    outw(0x604, 0x2000);   /* QEMU (modern) */
    outw(0xB004, 0x2000);  /* QEMU (older) / Bochs */
    outw(0x4004, 0x3400);  /* VirtualBox */

    __asm__ volatile ("cli");
    for (;;) {
        __asm__ volatile ("hlt");
    }
}