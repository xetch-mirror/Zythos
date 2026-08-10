bits 16

org 0x500

    global stage2_entry

KERNEL_STAGING_ADDR  equ 0x10000    ; временный адрес в реальном режиме (ниже 1MB)
KERNEL_FINAL_ADDR    equ 0x100000   ; финальный адрес ядра после защищённого режима
KERNEL_LBA           equ 16

%ifndef KERNEL_SECTORS
    %define KERNEL_SECTORS 16
%endif

stage2_entry:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    call enable_a20

    ; --- загрузить ядро в реальном режиме, ниже 1MB ---
    mov ax, KERNEL_STAGING_ADDR >> 4
    mov es, ax
    mov bx, 0
    mov ax, KERNEL_LBA
    mov cx, KERNEL_SECTORS
    call stage2_disk_read

    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp CODE_SEG:protected_mode_entry


enable_a20:
    push ax
    call wait_kbd_input_clear
    mov al, 0xAD
    out 0x64, al

    call wait_kbd_input_clear
    mov al, 0xD0
    out 0x64, al

    call wait_kbd_output_full
    in al, 0x60
    push ax

    call wait_kbd_input_clear
    mov al, 0xD1
    out 0x64, al

    call wait_kbd_input_clear
    pop ax
    or al, 2
    out 0x60, al

    call wait_kbd_input_clear
    mov al, 0xAE
    out 0x64, al

    call wait_kbd_input_clear
    pop ax
    ret

wait_kbd_input_clear:
    in al, 0x64
    test al, 2
    jnz wait_kbd_input_clear
    ret

wait_kbd_output_full:
    in al, 0x64
    test al, 1
    jz wait_kbd_output_full
    ret


bits 32

protected_mode_entry:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    cld
    mov esi, KERNEL_STAGING_ADDR
    mov edi, KERNEL_FINAL_ADDR
    mov ecx, (KERNEL_SECTORS * 512) / 4
    rep movsd

    mov esp, 0x90000
    call KERNEL_FINAL_ADDR

.halt:
    cli
    hlt
    jmp .halt

bits 16
%include "gdt.asm"
%include "disk.asm"