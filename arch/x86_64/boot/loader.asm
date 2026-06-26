; Copyright 2026 AiTOS authors.
; SPDX-License-Identifier: Apache-2.0

%include "boot.inc"

[bits 16]

org LOADER_BASE_ADDR
PM_STACK_TOP     equ 0x96000
LM_STACK_TOP     equ 0x1ff000

GDT_BASE:
    dd 0, 0
CODE32_DESC:
    dd 0x0000ffff
    dd 0x00cf9a00
DATA_DESC:
    dd 0x0000ffff
    dd 0x00cf9200
CODE64_DESC:
    dd 0x00000000
    dd 0x00af9a00
GDT_LIMIT equ $ - GDT_BASE - 1

gdt_ptr:
    dw GDT_LIMIT
    dd GDT_BASE

ards_buf  times 244 db 0
ards_nr   dw 0

loader_start:
    mov ax, 0x0003
    int 0x10
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0xf000

    xor ebx, ebx
    mov edx, 0x534d4150
    mov di, ards_buf
.e820_loop:
    mov eax, 0xe820
    mov ecx, 20
    int 0x15
    jc .e820_done
    add di, cx
    inc word [ards_nr]
    cmp di, loader_start
    jae .e820_done
    cmp ebx, 0
    jnz .e820_loop
.e820_done:
    mov cx, [ards_nr]
    jcxz .mem_default
    mov ebx, ards_buf
    xor edx, edx
.find_max:
    mov eax, [ebx]
    add eax, [ebx + 8]
    add ebx, 20
    cmp edx, eax
    jge .next_ards
    mov edx, eax
.next_ards:
    loop .find_max
    jmp .mem_store
.mem_default:
    mov edx, 32 * 1024 * 1024
.mem_store:
    mov dword [BOOT_INFO_ADDR], 0x535f494e
    mov dword [BOOT_INFO_ADDR + 4], 0x4149544f
    mov [BOOT_INFO_ADDR + 8], edx
    mov dword [BOOT_INFO_ADDR + 12], 0
    mov dword [BOOT_INFO_ADDR + 16], KERNEL_BIN_BASE_ADDR
    mov dword [BOOT_INFO_ADDR + 24], 0

    cli
    mov al, 0xff
    out 0x21, al
    out 0xa1, al

    in al, 0x92
    or al, 2
    out 0x92, al
    lgdt [gdt_ptr]
    mov eax, 0x11
    mov cr0, eax
    jmp dword SELECTOR_CODE32:pm_start

[bits 32]
pm_start:
    mov ax, SELECTOR_DATA
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, PM_STACK_TOP

    mov edi, PAGE_TABLE_BASE
    mov ecx, 512
    xor eax, eax
    cld
    rep stosd

    mov edi, PAGE_TABLE_BASE + 0x1000
    mov ecx, 512
    xor eax, eax
    rep stosd

    mov edi, PAGE_TABLE_BASE + 0x2000
    mov ecx, 512
    xor eax, eax
    rep stosd

    mov edi, PAGE_TABLE_BASE + 0x3000
    mov ecx, 512
    xor eax, eax
    rep stosd

    mov edi, PAGE_TABLE_BASE + 0x4000
    mov ecx, 512
    xor eax, eax
    rep stosd

    mov eax, PAGE_TABLE_BASE + 0x1000
    or eax, PAGE_PRESENT | PAGE_WRITE
    mov [PAGE_TABLE_BASE], eax

    mov eax, PAGE_TABLE_BASE + 0x2000
    or eax, PAGE_PRESENT | PAGE_WRITE
    mov [PAGE_TABLE_BASE + 0x1000], eax

    mov eax, PAGE_PRESENT | PAGE_WRITE | PAGE_SIZE_2M
    mov [PAGE_TABLE_BASE + 0x2000], eax

    mov eax, PAGE_TABLE_BASE + 0x3000
    or eax, PAGE_PRESENT | PAGE_WRITE
    mov [PAGE_TABLE_BASE + 511 * 8], eax

    mov eax, PAGE_TABLE_BASE + 0x4000
    or eax, PAGE_PRESENT | PAGE_WRITE
    mov [PAGE_TABLE_BASE + 0x3000 + 510 * 8], eax

    mov eax, KERNEL_BIN_BASE_ADDR
    or eax, PAGE_PRESENT | PAGE_WRITE | PAGE_SIZE_2M
    mov [PAGE_TABLE_BASE + 0x4000], eax

    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    mov ecx, 0xc0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    mov eax, PAGE_TABLE_BASE
    mov cr3, eax

    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    push dword SELECTOR_CODE64
    mov eax, long_mode_entry
    push eax
    retf

align 8
[bits 64]
long_mode_entry:
    mov rsp, strict qword LM_STACK_TOP
    and rsp, strict qword ~0xf
    mov rax, strict qword KERNEL_BIN_BASE_ADDR
    jmp rax
