; Copyright 2026 AiTOS authors.
; SPDX-License-Identifier: Apache-2.0
;
; Second-stage boot loader loaded at LOADER_BASE_ADDR (0x900) by the MBR.
; Probes memory (E820), fills boot_info, enables A20, enters 32-bit protected
; mode, builds a minimal 4-level page table, switches to long mode, and jumps
; to the raw kernel.bin entry at KERNEL_BIN_BASE_ADDR.

%include "boot.inc"						; physical addresses, page flags, GDT selectors

[bits 16]							; real-mode portion through loader_start

org LOADER_BASE_ADDR						; NASM origin: labels are physical addresses from 0x900
PM_STACK_TOP     equ 0x96000					; 32-bit protected-mode stack (physical)
LM_STACK_TOP     equ 0x1ff000					; temporary 64-bit stack before kernel sets RSP

; --- In-memory GDT for protected-mode and long-mode transitions ---
; Null descriptor plus 32-bit code/data and 64-bit long-mode code segment.
GDT_BASE:
    dd 0, 0							; entry 0: null descriptor (required)
CODE32_DESC:
    dd 0x0000ffff						; limit 0–15, base 0–15 (4 GiB flat 32-bit code)
    dd 0x00cf9a00						; base 16–31 + attributes (P,DPL0,code,readable)
DATA_DESC:
    dd 0x0000ffff						; limit 0–15, base 0–15 (4 GiB flat data)
    dd 0x00cf9200						; attributes (P,DPL0,expand-up data,writable)
CODE64_DESC:
    dd 0x00000000						; limit low (ignored in 64-bit mode with L=1)
    dd 0x00af9a00						; L=1 64-bit code segment, D=0, P=1
GDT_LIMIT equ $ - GDT_BASE - 1					; inclusive limit for LGDT

gdt_ptr:
    dw GDT_LIMIT						; limit field of 6-byte pseudo-descriptor
    dd GDT_BASE							; 32-bit linear address of GDT

ards_buf  times 244 db 0					; buffer for E820 memory map entries
ards_nr   dw 0							; count of ARDS entries collected

; ===========================================================================
; Real-mode entry (physical LOADER_ENTRY = LOADER_BASE_ADDR + 0x11c)
; ===========================================================================
loader_start:
    mov ax, 0x0003						; reset to 80x25 text mode after MBR handoff
    int 0x10							; BIOS video interrupt

    xor ax, ax							; AX = 0
    mov ds, ax							; DS = 0 for low-memory addressing
    mov es, ax							; ES = 0 for E820 buffer writes
    mov ss, ax							; SS = 0
    mov sp, 0xf000						; real-mode stack below boot structures

; --- E820 memory map probe (int 0x15, eax = 0xE820) ---
; Collects Address Range Descriptor entries into ards_buf until buffer full
; or BIOS returns continuation EBX = 0.
    xor ebx, ebx						; EBX = 0 starts enumeration
    mov edx, 0x534d4150						; 'SMAP' signature required by spec
    mov di, ards_buf						; DI = write pointer into ARDS buffer
.e820_loop:
    mov eax, 0xe820						; SMAP memory map function
    mov ecx, 20							; each ARDS entry is 20 bytes
    int 0x15							; BIOS memory services
    jc .e820_done						; carry = end of map or error
    add di, cx							; advance buffer pointer by entry size
    inc word [ards_nr]						; bump entry count
    cmp di, loader_start					; guard: do not overwrite code below loader_start
    jae .e820_done						; buffer full, stop probing
    cmp ebx, 0							; more entries?
    jnz .e820_loop						; EBX non-zero means continue enumeration
.e820_done:
    mov cx, [ards_nr]						; CX = number of map entries
    jcxz .mem_default						; no entries: use fallback size
    mov ebx, ards_buf						; EBX -> first ARDS entry
    xor edx, edx						; EDX = running maximum end address
.find_max:
    mov eax, [ebx]						; base address low 32 bits
    add eax, [ebx + 8]						; + length low 32 -> end of region
    add ebx, 20							; next ARDS entry
    cmp edx, eax						; current max >= this end?
    jge .next_ards						; skip update
    mov edx, eax						; new maximum RAM end address
.next_ards:
    loop .find_max						; process all entries
    jmp .mem_store						; store result in boot_info
.mem_default:
    mov edx, 32 * 1024 * 1024					; fallback: assume 32 MiB if E820 failed
.mem_store:
    mov dword [BOOT_INFO_ADDR], 0x535f494e			; magic 'IN_S' (boot_info)
    mov dword [BOOT_INFO_ADDR + 4], 0x4149544f			; magic 'OTIA' (AiTOS)
    mov [BOOT_INFO_ADDR + 8], edx				; total memory bytes
    mov dword [BOOT_INFO_ADDR + 12], 0				; reserved
    mov dword [BOOT_INFO_ADDR + 16], KERNEL_BIN_BASE_ADDR	; kernel phys base
    mov dword [BOOT_INFO_ADDR + 24], 0				; reserved

; --- Mask PIC, enable A20, load GDT, enter protected mode ---
    cli								; no interrupts until IDT exists in kernel
    mov al, 0xff						; mask all IRQs on master PIC
    out 0x21, al						; master PIC data port
    out 0xa1, al						; slave PIC data port

    in al, 0x92							; fast A20 gate (port 0x92)
    or al, 2							; set bit 1 to enable A20 line
    out 0x92, al						; write back to system control port A
    lgdt [gdt_ptr]						; load GDT built above
    mov eax, 0x11						; CR0: ET=1 (286 extension), PE=1 (protect enable)
    mov cr0, eax						; enter protected mode
    jmp dword SELECTOR_CODE32:pm_start				; far jump flushes prefetch, loads CS

; ===========================================================================
; 32-bit protected mode: build page tables and enable long mode
; ===========================================================================
[bits 32]
pm_start:
    mov ax, SELECTOR_DATA					; flat data segment selector
    mov ds, ax							; DS = 4 GiB data segment
    mov es, ax							; ES = data
    mov fs, ax							; FS = data
    mov gs, ax							; GS = data
    mov ss, ax							; SS = data
    mov esp, PM_STACK_TOP					; 32-bit stack for paging setup

; --- Zero five 4 KiB pages at PAGE_TABLE_BASE (PML4, PDPT, PD, PD, PD) ---
    mov edi, PAGE_TABLE_BASE					; EDI = start of page-table region
    mov ecx, 512						; 512 dwords = 2048 bytes (one page)
    xor eax, eax						; fill with zero
    cld								; rep stosd increments EDI
    rep stosd							; clear PML4 page at +0x0000

    mov edi, PAGE_TABLE_BASE + 0x1000				; PDPT page
    mov ecx, 512						; 512 dwords
    xor eax, eax						; zero value
    rep stosd							; clear PDPT

    mov edi, PAGE_TABLE_BASE + 0x2000				; low PD page (identity map)
    mov ecx, 512						; 512 dwords
    xor eax, eax						; zero value
    rep stosd							; clear identity PD

    mov edi, PAGE_TABLE_BASE + 0x3000				; high PDPT page (kernel half)
    mov ecx, 512						; 512 dwords
    xor eax, eax						; zero value
    rep stosd							; clear high PDPT

    mov edi, PAGE_TABLE_BASE + 0x4000				; high PD page (kernel 2 MiB map)
    mov ecx, 512						; 512 dwords
    xor eax, eax						; zero value
    rep stosd							; clear kernel PD

; --- Relocate kernel.bin from MBR staging to link address (32-bit, paging off) ---
    mov esi, KERNEL_STAGING_ADDR
    mov edi, KERNEL_BIN_BASE_ADDR
    mov ecx, KERNEL_COPY_BYTES / 4
    cld
    rep movsd

; --- Link 4-level hierarchy: identity 2 MiB at physical 0 ---
; PML4[0] -> PDPT@+0x1000 -> PD@+0x2000 -> 2 MiB page at phys 0
    mov eax, PAGE_TABLE_BASE + 0x1000				; physical address of PDPT
    or eax, PAGE_PRESENT | PAGE_WRITE				; present, supervisor writable
    mov [PAGE_TABLE_BASE], eax					; PML4 entry 0

    mov eax, PAGE_TABLE_BASE + 0x2000				; physical address of low PD
    or eax, PAGE_PRESENT | PAGE_WRITE				; present, writable
    mov [PAGE_TABLE_BASE + 0x1000], eax				; PDPT entry 0

    mov eax, PAGE_PRESENT | PAGE_WRITE | PAGE_SIZE_2M		; huge page at phys 0
    mov [PAGE_TABLE_BASE + 0x2000], eax				; PD entry 0: identity map 0–2 MiB

; --- High-half kernel map: PML4[511] -> kernel at KERNEL_BIN_BASE_ADDR ---
; Canonical kernel VA 0xFFFFFFFF80000000 uses PML4 index 511.
    mov eax, PAGE_TABLE_BASE + 0x3000				; high PDPT physical address
    or eax, PAGE_PRESENT | PAGE_WRITE				; present, writable
    mov [PAGE_TABLE_BASE + 511 * 8], eax			; PML4 entry 511

    mov eax, PAGE_TABLE_BASE + 0x4000				; high PD physical address
    or eax, PAGE_PRESENT | PAGE_WRITE				; present, writable
    mov [PAGE_TABLE_BASE + 0x3000 + 510 * 8], eax		; PDPT entry 510

    mov eax, KERNEL_BIN_BASE_ADDR				; physical base of loaded kernel.bin
    or eax, PAGE_PRESENT | PAGE_WRITE | PAGE_SIZE_2M		; 2 MiB huge page
    mov [PAGE_TABLE_BASE + 0x4000], eax				; map kernel phys -> high-half PD entry

; --- Enable PAE (required before long mode) ---
    mov eax, cr4						; read CR4
    or eax, 1 << 5						; set CR4.PAE (bit 5)
    mov cr4, eax						; write CR4 with PAE enabled

; --- Set IA32_EFER.LME (long mode enable) via MSR 0xC0000080 ---
    mov ecx, 0xc0000080						; IA32_EFER MSR number
    rdmsr							; EDX:EAX = current EFER
    or eax, 1 << 8						; set LME bit (bit 8)
    wrmsr							; write EFER with LME=1

    mov eax, PAGE_TABLE_BASE					; CR3 = physical address of PML4
    mov cr3, eax						; load page-table root

    mov eax, cr0						; read CR0
    or eax, 0x80000000						; set CR0.PG (paging enable)
    mov cr0, eax						; paging on; still in compatibility mode until far jump

; --- Far return to 64-bit code segment (loads 64-bit CS, enables long mode) ---
    push dword SELECTOR_CODE64					; push 64-bit code segment selector
    mov eax, long_mode_entry					; 32-bit offset of 64-bit entry point
    push eax							; push offset for far return
    retf							; pop CS:RIP -> long_mode_entry in 64-bit mode

align 8
; ===========================================================================
; 64-bit long mode: set stack and jump to raw kernel.bin entry
; ===========================================================================
[bits 64]
long_mode_entry:
    mov rsp, strict qword LM_STACK_TOP				; temporary 64-bit stack
    and rsp, strict qword ~0xf					; 16-byte align RSP (SysV ABI)
    mov rax, strict qword KERNEL_BIN_BASE_ADDR			; kernel linked for high half;
						; loader jumps to phys load addr
    jmp rax							; enter kernel _start (linked VMA applies)
