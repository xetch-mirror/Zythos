#ifndef VGA_H
#define VGA_H

#include "types.h"

#define VGA_ADDRESS 0xB8000
#define VGA_WIDTH   80
#define VGA_HEIGHT  25

/* пока не реализовано */
void vga_init(void);
void vga_putchar(char c);
void vga_clear(void);

#endif // VGA_H