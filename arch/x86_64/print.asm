; Copyright 2026 AiTOS authors.
; SPDX-License-Identifier: Apache-2.0

[bits 64]

VGA_MEM     equ 0xb8000
DEBUGCON    equ 0xe9
COM1_DATA   equ 0x3f8
VGA_WIDTH   equ 80
VGA_HEIGHT  equ 25

section .data
cursor_pos  dq 0

section .text
global arch_console_putchar
global arch_console_reset_cursor

arch_console_putchar:
    push rbx
    push rdi
    cld
    mov bl, dil

    mov dx, DEBUGCON
    mov al, bl
    out dx, al
    cmp bl, 0xa
    jne .com1
    mov al, 0xd
    out dx, al
.com1:
    mov dx, COM1_DATA
    mov al, bl
    out dx, al
    cmp bl, 0xa
    jne .vga
    mov al, 0xd
    out dx, al
.vga:
    cmp bl, 0xa
    je .newline
    cmp bl, 0xd
    je .done
    cmp bl, 0x8
    je .backspace

    mov rdi, VGA_MEM
    mov rcx, [cursor_pos]
    mov al, bl
    mov ah, 0x0F
    mov [rdi + rcx * 2], ax
    inc qword [cursor_pos]
    jmp .done

.backspace:
    cmp qword [cursor_pos], 0
    je .done
    dec qword [cursor_pos]
    mov rdi, VGA_MEM
    mov rcx, [cursor_pos]
    mov word [rdi + rcx * 2], 0x0F20
    jmp .done

.newline:
    mov rax, [cursor_pos]
    xor rdx, rdx
    mov rcx, VGA_WIDTH
    div rcx
    inc rax
    mul rcx
    mov [cursor_pos], rax
    cmp rax, VGA_WIDTH * VGA_HEIGHT
    jb .done
    xor rax, rax
    mov [cursor_pos], rax

.done:
    ; sync VGA hardware cursor to cursor_pos
    mov rcx, [cursor_pos]
    mov dx, 0x3d4
    mov al, 0x0f
    out dx, al
    mov dx, 0x3d5
    mov al, cl
    out dx, al
    mov dx, 0x3d4
    mov al, 0x0e
    out dx, al
    mov dx, 0x3d5
    mov al, ch
    out dx, al

    pop rdi
    pop rbx
    ret

arch_console_reset_cursor:
    mov qword [cursor_pos], 0
    ret
