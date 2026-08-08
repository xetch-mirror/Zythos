bits 16

section .rodata

align 8
gdt_start:
    ; нулевой дескриптор (обязателен)
    dq 0

gdt_code:
    dw 0xFFFF        ; limit (0-15)
    dw 0x0000        ; base (0-15)
    db 0x00          ; base (16-23)
    db 10011010b     ; access byte: code, readable
    db 11001111b     ; flags (4K granularity, 32-bit) + limit (16-19)
    db 0x00          ; base (24-31)

gdt_data:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b     ; access byte: data, writable
    db 11001111b
    db 0x00

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start