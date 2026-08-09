/* Обработчик IRQ1 (клавиатура) — читает scancode, декодирует через
   keyboard.c, кладёт печатаемые символы в kbd_buffer для sys_read() */
#include "ps2.h"
#include "keyboard.h"
#include "kbd_buffer.h"
#include "pic.h"

void irq1_handler(void)
{
    uint8_t scancode = ps2_read_data();
    keyboard_event_t ev;

    if (keyboard_handle_set1_scancode(scancode, &ev)) {
        if (ev.pressed && ev.is_text) {
            kbd_buffer_push((char)ev.codepoint);
        }
    }

    pic_send_eoi(1);
}