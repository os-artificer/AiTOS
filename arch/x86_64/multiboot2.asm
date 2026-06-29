; Copyright 2026 AiTOS authors.
; SPDX-License-Identifier: Apache-2.0
;
; Multiboot2 header + 32-bit entry stub.
;
; GRUB (BIOS) enters a Multiboot2 kernel in 32-bit protected mode with paging
; off and long mode disabled. AiTOS's _start (arch/x86_64/kernel.asm) is 64-bit
; code that assumes long mode is already active (the legacy loader.asm sets that
; up before jumping to _start). To boot under GRUB while keeping the legacy
; bootloader working, the Multiboot2 "entry address tag" (type 3) points GRUB at
; mb2_entry32 below. That stub builds a GDT + identity-mapped PAE page tables,
; enables long mode, and jumps to the shared 64-bit _start with EAX=magic and
; EBX=MBI preserved. The legacy path still enters _start directly (already in
; long mode) via the ELF entry / kernel.bin first byte, so it is unaffected.

%define MB2_MAGIC        0xE85250D6
%define MB2_ARCH         0
%define MB2_TAG_INFO     1
%define MB2_TAG_ENTRY    3
%define MB2_TAG_MMAP     6
%define MB2_TAG_END      0

%define PG_P             0x1
%define PG_W             0x2
%define PG_PS            0x80
%define EFER_MSR         0xc0000080
%define EFER_LME         (1 << 8)
%define CR4_PAE          (1 << 5)
%define CR0_PG           0x80000000

%define mb2_sel_code64   0x08
%define mb2_sel_data     0x10

extern _start

section .multiboot
align 8

header_start:
    dd MB2_MAGIC
    dd MB2_ARCH
    dd header_end - header_start
    dd -(MB2_MAGIC + MB2_ARCH + (header_end - header_start))

    align 8
    dw MB2_TAG_INFO
    dw 0
    dd 12
    dd MB2_TAG_MMAP

    ; Entry address tag: tell GRUB to jump to the 32-bit long-mode bringup stub
    ; instead of the 64-bit ELF entry (_start). entry_addr is a physical address.
    align 8
    dw MB2_TAG_ENTRY
    dw 0
    dd 12
    dd mb2_entry32

    align 8
    dw MB2_TAG_END
    dw 0
    dd 8

header_end:

; ===========================================================================
; 32-bit Multiboot2 entry: build long mode, then jump to the shared _start.
; On entry (per Multiboot2): EAX = bootloader magic, EBX = MBI physical addr,
; protected mode, paging off, flat 4 GiB segments, interrupts undefined.
; ===========================================================================
section .text
[bits 32]
global mb2_entry32
mb2_entry32:
    cli
    mov esi, eax                ; preserve Multiboot2 magic (EBX already holds MBI)
    mov esp, mb2_boot_stack_top ; private stack for the far return below

    lgdt [mb2_gdt_ptr]          ; load our GDT (CS reload happens at the far ret)

    ; --- Carve three 4 KiB-aligned page-table pages from mb2_pt_area ---
    mov ebp, mb2_pt_area
    add ebp, 0xfff
    and ebp, ~0xfff             ; EBP = PML4 (4 KiB aligned); PDPT=+0x1000, PD=+0x2000

    mov edi, ebp                ; zero PML4 + PDPT + PD (3 pages)
    mov ecx, 3 * 1024
    xor eax, eax
    cld
    rep stosd

    lea eax, [ebp + 0x1000]     ; PML4[0] -> PDPT
    or eax, PG_P | PG_W
    mov [ebp], eax

    lea eax, [ebp + 0x2000]     ; PDPT[0] -> PD
    or eax, PG_P | PG_W
    mov [ebp + 0x1000], eax

    lea edi, [ebp + 0x2000]     ; PD: identity map first 1 GiB with 2 MiB pages
    mov eax, PG_P | PG_W | PG_PS
    mov ecx, 512
.fill_pd:
    mov [edi], eax
    mov dword [edi + 4], 0
    add eax, 0x200000
    add edi, 8
    dec ecx
    jnz .fill_pd

    mov cr3, ebp                ; CR3 = PML4 physical

    mov eax, cr4
    or eax, CR4_PAE             ; PAE required before long mode
    mov cr4, eax

    mov ecx, EFER_MSR
    rdmsr
    or eax, EFER_LME            ; IA32_EFER.LME = 1
    wrmsr

    mov eax, cr0
    or eax, CR0_PG              ; enable paging -> long mode (compatibility) active
    mov cr0, eax

    push dword mb2_sel_code64   ; far return to 64-bit code segment
    push dword mb2_long
    retf

[bits 64]
mb2_long:
    mov ax, mb2_sel_data        ; reload data segments from our GDT
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov eax, esi                ; restore Multiboot2 magic for _start handoff check
    jmp _start                  ; shared 64-bit kernel entry (EBX = MBI preserved)

; ===========================================================================
; GDT for the long-mode bringup: null, 64-bit code (0x08), data (0x10).
; CS=0x08 keeps the kernel's grub_cs_selector()/console_is_serial() checks happy.
; ===========================================================================
section .rodata
align 8
mb2_gdt:
    dq 0x0000000000000000       ; 0x00: null
    dq 0x00af9a000000ffff       ; 0x08: 64-bit code (L=1, present, DPL0, exec/read)
    dq 0x00cf92000000ffff       ; 0x10: data (present, DPL0, read/write)
mb2_gdt_end:

mb2_gdt_ptr:
    dw mb2_gdt_end - mb2_gdt - 1
    dd mb2_gdt

; ===========================================================================
; Page-table scratch (in .data so _start's BSS clear does not wipe it while
; CR3 still points here). 4 pages give room to 4 KiB-align 3 used pages.
; ===========================================================================
section .data
align 4096
mb2_pt_area:
    times 4 * 4096 db 0

align 16
mb2_boot_stack:
    times 256 db 0
mb2_boot_stack_top:
