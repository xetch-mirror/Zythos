#ifndef KBD_BUFFER_H
#define KBD_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

void kbd_buffer_push(char c);
bool kbd_buffer_pop(char *out);   /* false, если буфер пуст */

#endif // KBD_BUFFER_H