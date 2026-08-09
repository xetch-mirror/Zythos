/* Перепрограммирование 8259 PIC — обязательно перед использованием IRQ,
   иначе IRQ0-7 накладываются на исключения CPU 0x08-0x0F */
#include "pic.h"
#include "io.h"

#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1

#define PIC_EOI   0x20

void pic_remap(void)
{
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);

    outb(PIC1_CMD, 0x11); /* ICW1: инициализация, каскадный режим */
    outb(PIC2_CMD, 0x11);

    outb(PIC1_DATA, 0x20); /* ICW2: IRQ0-7 -> векторы 0x20-0x27 */
    outb(PIC2_DATA, 0x28); /* ICW2: IRQ8-15 -> векторы 0x28-0x2F */

    outb(PIC1_DATA, 0x04); /* ICW3: у ведущего PIC ведомый на IRQ2 */
    outb(PIC2_DATA, 0x02); /* ICW3: номер каскада у ведомого */

    outb(PIC1_DATA, 0x01); /* ICW4: режим 8086 */
    outb(PIC2_DATA, 0x01);

    outb(PIC1_DATA, mask1); /* восстановить маски */
    outb(PIC2_DATA, mask2);
}

void pic_send_eoi(uint8_t irq)
{
    if (irq >= 8) {
        outb(PIC2_CMD, PIC_EOI);
    }
    outb(PIC1_CMD, PIC_EOI);
}

void pic_set_mask(uint8_t irq)
{
    uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    uint8_t bit = (irq < 8) ? irq : irq - 8;
    uint8_t value = inb(port) | (1 << bit);
    outb(port, value);
}

void pic_clear_mask(uint8_t irq)
{
    uint16_t port = (irq < 8) ? PIC1_DATA : PIC2_DATA;
    uint8_t bit = (irq < 8) ? irq : irq - 8;
    uint8_t value = inb(port) & ~(1 << bit);
    outb(port, value);
}