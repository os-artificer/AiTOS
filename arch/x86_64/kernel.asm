; Copyright 2026 AiTOS authors.
; SPDX-License-Identifier: Apache-2.0

[bits 64]

extern irq_dispatch

EARLY_STACK_TOP equ 0x1ff000

%macro PUSH_REGS 0
    push r15
    push r14
    push r13
    push r12
    push r11
    push r10
    push r9
    push r8
    push rbp
    push rbx
    push rdx
    push rcx
    push rax
%endmacro

%macro POP_REGS 0
    pop rax
    pop rcx
    pop rdx
    pop rbx
    pop rbp
    pop r8
    pop r9
    pop r10
    pop r11
    pop r12
    pop r13
    pop r14
    pop r15
%endmacro

%macro ISR_NOERR 1
global intr%1_entry
intr%1_entry:
    push 0
    PUSH_REGS
    mov rdi, %1
    sub rsp, 8
    call irq_dispatch
    add rsp, 8
    POP_REGS
    add rsp, 8
    iretq
%endmacro

%macro ISR_ERR 1
global intr%1_entry
intr%1_entry:
    PUSH_REGS
    mov rdi, %1
    sub rsp, 8
    call irq_dispatch
    add rsp, 8
    POP_REGS
    add rsp, 8
    iretq
%endmacro

section .head.text
global _start
extern kmain
extern __bss_start
extern __bss_end

_start:
    cli
    mov rsp, EARLY_STACK_TOP
    and rsp, ~0xf
    mov rdi, __bss_start
    mov rcx, __bss_end
    sub rcx, rdi
    xor eax, eax
    cld
    rep stosb
    call kmain
.start_hang:
    hlt
    jmp .start_hang

ISR_NOERR 0x00
ISR_NOERR 0x01
ISR_NOERR 0x02
ISR_NOERR 0x03
ISR_NOERR 0x04
ISR_NOERR 0x05
ISR_NOERR 0x06
ISR_NOERR 0x07
ISR_ERR   0x08
ISR_NOERR 0x09
ISR_ERR   0x0a
ISR_ERR   0x0b
ISR_ERR   0x0c
ISR_ERR   0x0d
ISR_ERR   0x0e
ISR_NOERR 0x0f
ISR_NOERR 0x10
ISR_ERR   0x11
ISR_NOERR 0x12
ISR_NOERR 0x13
ISR_NOERR 0x14
ISR_NOERR 0x15
ISR_NOERR 0x16
ISR_NOERR 0x17
ISR_NOERR 0x18
ISR_NOERR 0x19
ISR_NOERR 0x1a
ISR_NOERR 0x1b
ISR_NOERR 0x1c
ISR_NOERR 0x1d
ISR_ERR   0x1e
ISR_NOERR 0x1f
ISR_NOERR 0x20
ISR_NOERR 0x21
ISR_NOERR 0x22
ISR_NOERR 0x23
ISR_NOERR 0x24
ISR_NOERR 0x25
ISR_NOERR 0x26
ISR_NOERR 0x27
ISR_NOERR 0x28
ISR_NOERR 0x29
ISR_NOERR 0x2a
ISR_NOERR 0x2b
ISR_NOERR 0x2c
ISR_NOERR 0x2d
ISR_NOERR 0x2e
ISR_NOERR 0x2f

section .rodata
global intr_entry_table
intr_entry_table:
    dq intr0x00_entry
    dq intr0x01_entry
    dq intr0x02_entry
    dq intr0x03_entry
    dq intr0x04_entry
    dq intr0x05_entry
    dq intr0x06_entry
    dq intr0x07_entry
    dq intr0x08_entry
    dq intr0x09_entry
    dq intr0x0a_entry
    dq intr0x0b_entry
    dq intr0x0c_entry
    dq intr0x0d_entry
    dq intr0x0e_entry
    dq intr0x0f_entry
    dq intr0x10_entry
    dq intr0x11_entry
    dq intr0x12_entry
    dq intr0x13_entry
    dq intr0x14_entry
    dq intr0x15_entry
    dq intr0x16_entry
    dq intr0x17_entry
    dq intr0x18_entry
    dq intr0x19_entry
    dq intr0x1a_entry
    dq intr0x1b_entry
    dq intr0x1c_entry
    dq intr0x1d_entry
    dq intr0x1e_entry
    dq intr0x1f_entry
    dq intr0x20_entry
    dq intr0x21_entry
    dq intr0x22_entry
    dq intr0x23_entry
    dq intr0x24_entry
    dq intr0x25_entry
    dq intr0x26_entry
    dq intr0x27_entry
    dq intr0x28_entry
    dq intr0x29_entry
    dq intr0x2a_entry
    dq intr0x2b_entry
    dq intr0x2c_entry
    dq intr0x2d_entry
    dq intr0x2e_entry
    dq intr0x2f_entry
