bits 16

; ------------------------------------------------------
; Читает сектора с диска в реальном режиме (BIOS int 13h)
; Параметры:
;   ax  - начальный LBA
;   cl  - количество секторов
;   dl  - номер диска
;   es:bx - куда записать данные
; ------------------------------------------------------
global stage2_disk_read
stage2_disk_read:
    mov [kernel_dap.lba], ax
    mov [kernel_dap.count], cx
    mov [kernel_dap.offset], bx
    mov [kernel_dap.segment], es

    mov si, kernel_dap
    mov ah, 0x42
    int 0x13
    jc .fail
    ret

.fail:
    cli
.halt:
    hlt
    jmp .halt

kernel_dap:
    db 0x10
    db 0
    .count:   dw 0
    .offset:  dw 0
    .segment: dw 0
    .lba:     dq 0