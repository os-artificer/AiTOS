; Copyright 2026 AiTOS authors.
;
; Licensed under the Apache License, Version 2.0 (the "License");
; you may not use this file except in compliance with the License.
; You may obtain a copy of the License at
;
;     http://www.apache.org/licenses/LICENSE-2.0
;
; Unless required by applicable law or agreed to in writing, software
; distributed under the License is distributed on an "AS IS" BASIS,
; WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
; See the License for the specific language governing permissions and
; limitations under the License.

%include "boot.inc"

[org  0x7C00]
[bits 16]

SECTION MBR vstart=0x7c00
    mov  ax, cs
    mov  ds, ax
    mov  es, ax
    mov  ss, ax
    mov  fs, ax
    mov  sp, 0x7c00
    mov  ax, 0xb800 ;add to operate the graphics memory,and 0xb8000 is the begining of the graphics memory
    mov  gs, ax

    ;clear the screen, use the 0x10 interrupt
    ;AH      = 0x06(function number)
    ;AL      = the upScroll's row number
    ;BH      = the attribute of the upScroll
    ;(CL,CH) = (x,y) in the upper left corner of windows
    ;(DL,DH) = (x,y) int the lower right corner of windows

    mov  ax, 0x0600
    mov  bx, 0x0700
    mov  cx, 0       ;the upper left corner (0,0)
    mov  dx, 0x184f  ;the lower right corner (24,79)  (0x18 = 24,0x4f=79)
    int  0x10

    ;begin to load the loader
    ;operate the io

    mov  eax, LOADER_START_SECTOR
    mov  bx,  LOADER_BASE_ADDR
    mov  cx,  4
    call rd_disk_m_16
    jmp  LOADER_BASE_ADDR + 0x300

rd_disk_m_16:
    ;copy the value
    mov  esi, eax
    mov  di,  cx
    mov  al,  cl
    mov  dx,  0x1F2
    out  dx,  al
    mov  eax, esi   ;recover the value of ax
    mov  dx,  0x1F3
    out  dx,  al
    mov  cl,  0x8
    shr  eax, cl
    mov  dx,  0x1F4
    out  dx,  al
    shr  eax, cl
    mov  dx,  0x1F5
    out  dx,  al
    shr  eax, cl
    and  al,  0x0f
    or   al,  0xe0  ;11100000 = 0xe0,
    mov  dx,  0x1F6
    out  dx,  al
    mov  ax,  0x20
    mov  dx,  0x1F7
    out  dx,  al

.not_ready:
    nop
    in   al, dx
    and  al, 0x88
    cmp  al, 0x08
    jnz  .not_ready

    ;begine to loop read
    ;set the source
    mov  ax, di
    mov  dx, 256
    mul  dx
    mov  cx, ax
    mov  dx, 0x1F0

.loop_read_data:
    in   ax,   dx
    mov  [bx], ax
    add  bx,   2
    loop .loop_read_data
    ;read over
    ret

times 510-($-$$) db 0
db    0x55,0xaa
