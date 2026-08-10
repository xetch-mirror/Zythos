OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)
ENTRY(main)

KERNEL_LOAD_ADDR = 0x100000;

SECTIONS
{
    . = KERNEL_LOAD_ADDR;
    __kernel_start = .;

    .text ALIGN(0x1000) : {
        __text_start = .;
        *(.text .text.*)
        __text_end = .;
    }

    .rodata ALIGN(0x1000) : {
        __rodata_start = .;
        *(.rodata .rodata.*)
        __rodata_end = .;
    }

    .data ALIGN(0x1000) : {
        __data_start = .;
        *(.data .data.*)
        __data_end = .;
    }

    .bss ALIGN(0x1000) (NOLOAD) : {
        __bss_start = .;
        *(COMMON)
        *(.bss .bss.*)
        __bss_end = .;
    }

    . = ALIGN(0x1000);
    __kernel_end = .;

    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
        *(.note .note.*)
    }
}