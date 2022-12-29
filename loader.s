global loader
global a_out
global a_in
global a_lgdt

extern main


MAGIC_NUMBER      equ 0x1BADB002
FLAGS             equ 0x0
CHECKSUM          equ -MAGIC_NUMBER
KERNEL_STACK_SIZE equ 4096

section .bss
align 4
kernel_stack:
    resb KERNEL_STACK_SIZE

section .text:
align 4
    dd MAGIC_NUMBER
    dd FLAGS
    dd CHECKSUM

a_out:
    mov al, [esp + 8]  ; data to be sent
    mov dx, [esp + 4]  ; address
    out dx, al
    ret

a_in:
    mov dx, [esp + 4]  ; address
    in  al, dx
    ret

a_lgdt:
    lgdt [esp + 4]
    ret

; a_in:
;     mov al, [esp + 8]  ; data to be sent
;     mov dx, [esp + 4]  ; address
;     out dx, al
;     ret

loader:
    mov  esp, kernel_stack + KERNEL_STACK_SIZE
    call main
    ; mov ax, 0x10
    ; mov ds, ax
    ; mov ss, ax
    ; mov es, ax
    ; jmp 0x08:flush_cs
flush_cs:

.loop:
    jmp  .loop

; vim:ft=nasm
