#ifndef PIC_H
#define PIC_H

#include <stdint.h>

/* Перепрограммирует 8259 PIC так, чтобы IRQ0-15 не пересекались
   с зарезервированными исключениями CPU (0-31) */
void pic_remap(void);
void pic_send_eoi(uint8_t irq);
void pic_set_mask(uint8_t irq);
void pic_clear_mask(uint8_t irq);

#endif // PIC_H