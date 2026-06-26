; Copyright 2026 AiTOS authors.
; SPDX-License-Identifier: Apache-2.0

%macro SHOW_LOGO_LINE 2
%%line_loop:
    mov  al, [%1]
    cmp  al, 0
    je   %%line_done
    mov  ah, 0x0F
    mov  bx, %2
    mov  word [gs:bx], ax
    inc  %1
    add  %2, 2
    jmp  %%line_loop
%%line_done:
%endmacro

show_boot_logo:
    mov  ax, 0xb800
    mov  gs, ax
    mov  si, logo_line1
    mov  bx, 0
    SHOW_LOGO_LINE si, bx
    mov  si, logo_line2
    mov  bx, 160
    SHOW_LOGO_LINE si, bx
    ret

logo_line1: db "  AiTOS x86-64  ", 0
logo_line2: db "  Booting...    ", 0
