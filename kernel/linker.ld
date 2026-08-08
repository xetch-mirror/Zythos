ENTRY(_start)

SECTIONS
{
    . = 0x100000;   /* совпадает с KERNEL_FINAL_ADDR в stage2.asm */

    .text : {
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(COMMON)
        *(.bss)
    }
}