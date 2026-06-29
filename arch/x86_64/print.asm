; Copyright 2026 AiTOS authors.
; SPDX-License-Identifier: Apache-2.0
;
; Low-level console output for the x86-64 kernel. Mirrors each character to
; QEMU debugcon (port 0xE9), COM1 serial, and VGA text memory at 0xB8000.
; Maintains a software cursor and syncs the VGA hardware cursor via CRTC.

[bits 64]				; long-mode routines called from C
default rel				; RIP-relative addressing for .data symbols

extern console_vga_enabled		; set after vga_init_text_mode (legacy only)
extern boot_handoff_source		; 1 = Multiboot2 / GRUB headless path

BOOT_SOURCE_MULTIBOOT2 equ 1
VGA_MEM     equ 0xb8000			; linear address of color text frame buffer
DEBUGCON    equ 0xe9			; QEMU/Bochs debug console I/O port
COM1_DATA   equ 0x3f8			; UART 8250 transmit holding register
VGA_WIDTH   equ 80			; characters per text row
VGA_HEIGHT  equ 25			; text rows on screen

section .bss
cursor_pos  resq 1			; software cursor (BSS — must not overlap boot_stack_top)

section .text
global arch_console_putchar		; void arch_console_putchar(char c)
global arch_console_reset_cursor	; void arch_console_reset_cursor(void)
global debugcon_puts			; void debugcon_puts(const char *s)
extern console_puts_legacy		; void console_puts_legacy(const char *s)

; void console_puts(const char *s)
global console_puts
console_puts:
    push rbp
    mov rbp, rsp
    push rbx
    mov rbx, rdi
    cmp qword [rel boot_handoff_source], BOOT_SOURCE_MULTIBOOT2
    je .debugcon
    mov ax, cs
    cmp ax, 0x10
    je .debugcon
    cmp ax, 0x8
    je .debugcon
    jmp .legacy
.debugcon:
    mov rdi, rbx
    call debugcon_puts
    jmp .done
.legacy:
    mov rdi, rbx
    call console_puts_legacy
.done:
    pop rbx
    pop rbp
    ret

; void debugcon_puts(const char *s) — debugcon only (GRUB headless path)
debugcon_puts:
    push rbx
    test rdi, rdi
    jz .done
    mov rbx, rdi
.loop:
    mov al, [rbx]
    test al, al
    jz .done
    out 0xe9, al
    cmp al, 0xa
    jne .next
    mov al, 0xd
    out 0xe9, al
.next:
    inc rbx
    jmp .loop
.done:
    pop rbx
    ret

; --- Output one character to debugcon, serial, and VGA text buffer ---
; dil = character (SysV ABI low byte of first integer arg)
arch_console_putchar:
    push rbx				; preserve RBX (callee-saved)
    push rdi				; preserve RDI (callee-saved)
    cld					; C code expects DF=0
    mov bl, dil				; BL = character to output

; --- Mirror to QEMU debugcon (single-byte port write) ---
    mov dx, DEBUGCON			; debugcon port
    mov al, bl				; character to write
    out dx, al				; emit to host stdio when -debugcon is set
    cmp bl, 0xa				; newline?
    jne .com1				; skip CR if not LF
    mov al, 0xd				; CR for debugcon LF expansion
    out dx, al				; write carriage return

; --- Mirror to COM1 serial (same LF -> CR+LF convention) ---
.com1:
    mov dx, COM1_DATA			; UART data port
    mov al, bl				; character to transmit
    out dx, al				; send byte to serial
    cmp bl, 0xa				; newline?
    jne .com1_done			; skip CR if not LF
    mov al, 0xd				; carriage return before line feed
    out dx, al				; complete CRLF on serial
.com1_done:
    cmp byte [rel console_vga_enabled], 0
    je .early_done			; default: debugcon + COM1 only

; --- VGA text buffer: handle control characters and printable glyphs ---
.vga:
    cmp bl, 0xa				; LF: advance to next row
    je .newline				; handle line feed
    cmp bl, 0xd				; CR: ignore (software cursor handles rows)
    je .done				; nothing to draw
    cmp bl, 0x8				; BS: backspace
    je .backspace			; erase previous cell

; --- Write printable character at current cursor cell ---
    mov rdi, VGA_MEM			; RDI = base of VGA text buffer
    mov rcx, [cursor_pos]		; RCX = cell index (NOT the character)
    mov al, bl				; AL = ASCII character
    mov ah, 0x0F			; AH = bright white on black attribute
    mov [rdi + rcx * 2], ax		; store char+attr (2 bytes per cell)
    inc qword [cursor_pos]		; advance to next cell
    jmp .done				; skip special-case handlers

; --- Backspace: move cursor left and blank the cell ---
.backspace:
    cmp qword [cursor_pos], 0		; already at home?
    je .done				; ignore backspace at origin
    dec qword [cursor_pos]		; move cursor back one cell
    mov rdi, VGA_MEM			; VGA base address
    mov rcx, [cursor_pos]		; index of cell to erase
    mov word [rdi + rcx * 2], 0x0F20	; space with white-on-black attr
    jmp .done				; finished backspace

; --- Newline: move cursor to column 0 of next row (wrap at screen end) ---
.newline:
    mov rax, [cursor_pos]		; current cell index
    xor rdx, rdx			; clear high div result
    mov rcx, VGA_WIDTH			; divide by columns per row
    div rcx				; RAX = row, RDX = column
    inc rax				; next row number
    mul rcx				; RAX = row * WIDTH = cell index of row start
    mov [cursor_pos], rax		; store new cursor position
    cmp rax, VGA_WIDTH * VGA_HEIGHT	; past last cell?
    jb .done				; still on screen
    xor rax, rax			; wrap to top-left
    mov [cursor_pos], rax		; cursor = 0

.early_done:
    pop rdi				; restore RDI
    pop rbx				; restore RBX
    ret					; return to C caller

.done:
; --- Sync VGA hardware cursor (CRTC ports 0x3D4 index / 0x3D5 data) ---
; cursor_pos is a cell index; split into low/high bytes for CRTC registers.
    mov rcx, [cursor_pos]		; RCX = cell index for hardware cursor
    mov dx, 0x3d4			; CRTC address register
    mov al, 0x0f			; register 0x0F = cursor location low byte
    out dx, al				; select CRTC register
    mov dx, 0x3d5			; CRTC data register
    mov al, cl				; low 8 bits of cursor position
    out dx, al				; write cursor low byte
    mov dx, 0x3d4			; CRTC address register again
    mov al, 0x0e			; register 0x0E = cursor location high byte
    out dx, al				; select high-byte register
    mov dx, 0x3d5			; CRTC data register
    mov al, ch				; high 8 bits of cursor position
    out dx, al				; write cursor high byte

    pop rdi				; restore RDI
    pop rbx				; restore RBX
    ret					; return to C caller

; --- Reset software cursor to top-left (used by shell clear command) ---
arch_console_reset_cursor:
    mov qword [cursor_pos], 0		; cell index 0
    ret					; return to C
