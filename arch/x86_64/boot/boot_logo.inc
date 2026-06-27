; Copyright 2026 AiTOS authors.
; SPDX-License-Identifier: Apache-2.0
;
; VGA text-mode boot logo for real-mode MBR. Writes white-on-black characters
; to segment 0xB800 (physical 0xB8000) using GS-relative addressing.

; --- Macro: draw one null-terminated string at a VGA text row ---
; %1 = SI pointer to string, %2 = byte offset from start of screen (row * 160)
%macro SHOW_LOGO_LINE 2
%%line_loop:
    mov  al, [%1]			; load next character from string
    cmp  al, 0				; end of null-terminated string?
    je   %%line_done			; yes: line complete
    mov  ah, 0x0F			; attribute: bright white on black
    mov  bx, %2				; VGA cell offset (column*2 within row base)
    mov  word [gs:bx], ax		; store char+attribute as 16-bit VGA cell
    inc  %1				; advance string pointer
    add  %2, 2				; advance to next column (2 bytes per cell)
    jmp  %%line_loop			; draw next character
%%line_done:
%endmacro

; --- Draw two-line logo at top of VGA text screen ---
show_boot_logo:
    mov  ax, 0xb800			; GS segment = 0xB800 -> linear 0xB8000
    mov  gs, ax				; point GS at VGA text buffer
    mov  si, logo_line1			; first line string address
    mov  bx, 0				; row 0, column 0 (offset 0)
    SHOW_LOGO_LINE si, bx		; render line 1
    mov  si, logo_line2			; second line string address
    mov  bx, 160			; row 1 offset (80 cols * 2 bytes = 160)
    SHOW_LOGO_LINE si, bx		; render line 2
    ret					; return to MBR

logo_line1: db "  AiTOS x86-64  ", 0	; title line (null-terminated)
logo_line2: db "  Booting...    ", 0	; status line (null-terminated)
