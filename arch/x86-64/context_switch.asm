bits 32
section .text
global context_switch

; void context_switch(uint32_t *old_esp_store, uint32_t new_esp)
context_switch:
    push ebp
    push ebx
    push esi
    push edi

    mov eax, [esp + 20]   ; old_esp_store
    mov [eax], esp

    mov esp, [esp + 24]   ; new_esp

    pop edi
    pop esi
    pop ebx
    pop ebp
    ret