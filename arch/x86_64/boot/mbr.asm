; Copyright 2026 AiTOS authors.
; SPDX-License-Identifier: Apache-2.0
;
; Master Boot Record (512 bytes at LBA 0). BIOS loads this sector to 0x7C00,
; sets CS:IP, and executes in real mode. Loads loader + kernel via primary IDE
; PIO (LBA28), shows a VGA logo, then jumps to the loader entry point.

%include "boot.inc"			; disk layout constants and GDT selectors

[org 0x7c00]				; BIOS loads this sector at physical 0x7C00
[bits 16]				; real mode: 16-bit operands and addresses

; --- Real-mode segment setup and VGA text mode ---
; BIOS leaves CS=0x0000 and IP=0x7C00. Flatten all data segments to CS so
; offsets in this sector address correctly without separate data segment base.
    mov ax, cs				; AX = current code segment (0x0000 after BIOS load)
    mov ds, ax				; DS = CS for data references in this segment
    mov es, ax				; ES = CS (used by some string/disk routines)
    mov ss, ax				; SS = CS for stack below 0x7C00
    mov fs, ax				; FS = CS (unused here, kept consistent)
    mov sp, 0x7c00			; stack grows downward from load address
    mov ax, 0xb800			; GS -> VGA text buffer at 0xB8000
    mov gs, ax				; GS segment for boot_logo.inc writes

    mov ax, 0x0003			; AH=0x00 AL=0x03: set video mode 80x25 color text
    int 0x10				; BIOS video services

    call show_boot_logo			; draw logo via boot_logo.inc (included below)

; --- Load loader.bin (4 sectors starting at LBA 2) to 0x900 ---
    mov eax, LOADER_START_SECTOR	; LBA of first loader sector
    mov bx, LOADER_BASE_ADDR		; destination offset (segment DS=0)
    mov cx, 4				; sector count for loader image
    call rd_disk_m_16			; PIO read loop

; --- Wait for primary IDE channel not busy before second transfer ---
    mov dx, 0x1f7			; primary status register
.wait_bsy:
    in al, dx				; read status byte
    test al, 0x80			; BSY bit set?
    jnz .wait_bsy			; wait until drive ready

; --- Load kernel.bin to physical 0x70000 (DS:BX = 0x7000:0) ---
    push ds				; save MBR data segment
    mov ax, KERNEL_BIN_BASE_ADDR >> 4	; paragraph base for 0x70000
    mov ds, ax				; DS = 0x7000 -> linear 0x70000
    xor bx, bx				; offset 0 within segment
    mov eax, KERNEL_START_SECTOR	; LBA 6: first kernel sector
    mov cx, KERNEL_SECTOR_COUNT		; number of 512-byte sectors
    call rd_disk_m_16			; read kernel image
    pop ds				; restore DS=0 for loader jump

    jmp LOADER_ENTRY			; transfer control to loader_start

; --- Multi-sector PIO read wrapper (real mode) ---
; eax = starting LBA, bx = destination offset, cx = sector count
; Uses DS as segment for [bx] stores (DS:BX linear address).
rd_disk_m_16:
    mov si, cx				; SI = remaining sector count
    mov di, bx				; DI = running destination offset
    mov bp, ax				; BP = running LBA
.next:
    mov bx, di				; current write offset for this sector
    mov ax, bp				; current LBA for this sector
    mov cx, 1				; rd_disk_one reads one sector at a time
    call rd_disk_one			; read single sector via IDE
    add di, 512				; advance destination by one sector
    inc bp				; next LBA
    dec si				; sectors remaining
    jnz .next				; read next sector if any
    mov bx, di				; return final offset in BX
    ret

; --- Single-sector LBA28 read on primary IDE (ports 0x1F0–0x1F7) ---
; cl = sector count (1), eax = LBA, bx = buffer offset in DS segment
rd_disk_one:
    push bx				; preserve buffer offset
    push ax				; preserve LBA low 16 bits

    mov al, cl				; sector count to write to 0x1F2
    mov dx, 0x1f2			; sector count register
    out dx, al				; program sector count

    pop ax				; restore LBA
    push ax				; save again for high-byte writes
    mov dx, 0x1f3			; LBA bits 0–7
    out dx, al				; write LBA low byte
    mov al, ah				; LBA bits 8–15
    mov dx, 0x1f4			; LBA mid register
    out dx, al				; write LBA mid byte
    pop ax				; restore full 16-bit LBA
    shr ax, 8				; shift to get bits 16–23 in AL
    mov dx, 0x1f5			; LBA high register
    out dx, al				; write LBA bits 16–23
    shr ax, 8				; clear upper bits of AL
    and al, 0x0f			; LBA bits 24–27 in low nibble
    or al, 0xe0				; drive/head: master, LBA mode (0xE0)
    mov dx, 0x1f6			; drive/head register
    out dx, al				; write drive select + LBA top bits

    pop bx				; restore buffer offset

    mov al, 0x20			; READ SECTORS (PIO) command
    mov dx, 0x1f7			; command/status register
    out dx, al				; issue read command
.wait:
    in al, dx				; poll status
    test al, 0x80			; BSY still set?
    jnz .wait				; wait for not busy
    and al, 0x08			; DRQ (data request) bit
    cmp al, 0x08			; data ready for PIO?
    jne .wait				; wait until DRQ asserted

    mov cx, 256				; 256 words = 512 bytes per sector
    mov dx, 0x1f0			; data port: read 16-bit words from drive
.read:
    in ax, dx				; read one word from IDE data register
    mov [bx], ax			; store to DS:BX buffer
    add bx, 2				; advance buffer by word
    loop .read				; CX times (256 words)
    ret

%include "boot_logo.inc"		; show_boot_logo and logo strings

; --- Boot sector size padding and 0xAA55 signature ---
times 510 - ($ - $$) db 0		; pad to byte 510 (510 - current size)
db 0x55, 0xaa				; boot signature at bytes 510–511
