bits 32
section .text
global irq1_stub
extern irq1_handler

irq1_stub:
    pusha
    call irq1_handler
    popa
    iret