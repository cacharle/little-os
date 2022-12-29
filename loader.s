global loader
global a_out
global a_in
global a_lgdt

extern main


MAGIC_NUMBER      equ 0x1BADB002
FLAGS             equ 0x0
CHECKSUM          equ -MAGIC_NUMBER
KERNEL_STACK_SIZE equ 4096

gdt:
    dd 0
    dd 0 ; null desc
code_descriptor:
    dw 0xffff
    dw 0
    db 0
    db 0b10011010
    db 0b11001111
    db 0
data_descriptor:
    dw 0xffff
    dw 0
    db 0
    db 0b10010010
    db 0b11001111
    db 0
gdt_end

gdt_desc:
    dw gdt_end - gdt - 1
    dd gdt

section .data
align 4

CODE_SEG equ code_descriptor - gdt
DATA_SEG equ data_descriptor - gdt

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
    cli
    ; mov  ax, [esp + 4]
    ; mov [gdtr], ax
    ; mov eax, [esp + 8]
    ; add eax, [esp + 12]
    ; mov [gdtr + 2], eax
    ; lgdt [gdtr]
    lgdt [gdt_desc]
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp CODE_SEG:flush_cs
[bits 32]
flush_cs:
    ; mov ax, 0x10
    ; mov ds, ax
    ; mov es, ax
    ; mov fs, ax
    ; mov gs, ax
    ; mov ss, ax
    ret

; a_in:
;     mov al, [esp + 8]  ; data to be sent
;     mov dx, [esp + 4]  ; address
;     out dx, al
;     ret

loader:
    mov  esp, kernel_stack + KERNEL_STACK_SIZE
    call a_lgdt
    mov eax, 0xdeadbeef
    ; call main

.loop:
    jmp  .loop

; vim:ft=nasm
