; Copyright 2026 AiTOS authors.
; SPDX-License-Identifier: Apache-2.0

%include "boot.inc"

[org 0x7c00]
[bits 16]

    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov sp, 0x7c00
    mov ax, 0xb800
    mov gs, ax

    mov ax, 0x0003
    int 0x10

    call show_boot_logo

    mov eax, LOADER_START_SECTOR
    mov bx, LOADER_BASE_ADDR
    mov cx, 4
    call rd_disk_m_16

    mov dx, 0x1f7
.wait_bsy:
    in al, dx
    test al, 0x80
    jnz .wait_bsy

    push ds
    mov ax, KERNEL_BIN_BASE_ADDR >> 4
    mov ds, ax
    xor bx, bx
    mov eax, KERNEL_START_SECTOR
    mov cx, KERNEL_SECTOR_COUNT
    call rd_disk_m_16
    pop ds

    jmp LOADER_ENTRY

; eax = LBA, bx = offset, cx = sector count
rd_disk_m_16:
    mov si, cx
    mov di, bx
    mov bp, ax
.next:
    mov bx, di
    mov ax, bp
    mov cx, 1
    call rd_disk_one
    add di, 512
    inc bp
    dec si
    jnz .next
    mov bx, di
    ret

rd_disk_one:
    push bx
    push ax

    mov al, cl
    mov dx, 0x1f2
    out dx, al

    pop ax
    push ax
    mov dx, 0x1f3
    out dx, al
    mov al, ah
    mov dx, 0x1f4
    out dx, al
    pop ax
    shr ax, 8
    mov dx, 0x1f5
    out dx, al
    shr ax, 8
    and al, 0x0f
    or al, 0xe0
    mov dx, 0x1f6
    out dx, al

    pop bx

    mov al, 0x20
    mov dx, 0x1f7
    out dx, al
.wait:
    in al, dx
    test al, 0x80
    jnz .wait
    and al, 0x08
    cmp al, 0x08
    jne .wait

    mov cx, 256
    mov dx, 0x1f0
.read:
    in ax, dx
    mov [bx], ax
    add bx, 2
    loop .read
    ret

%include "boot_logo.inc"

times 510 - ($ - $$) db 0
db 0x55, 0xaa
