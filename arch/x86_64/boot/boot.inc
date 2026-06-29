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
;
; Boot-time physical memory layout and disk sector constants shared by MBR
; and loader. All addresses are physical unless noted as a kernel VMA.

; --- Loader placement on disk and in low memory ---
LOADER_BASE_ADDR	equ	0x900			; physical load address of loader.bin (sector 2+)
LOADER_START_SECTOR	equ	0x2			; LBA of first loader sector (after MBR)

; --- Kernel image placement ---
KERNEL_START_SECTOR	equ	0x6			; LBA where kernel.bin begins on the disk
KERNEL_SECTOR_COUNT	equ	40			; sectors read by MBR PIO (20 KiB)
KERNEL_STAGING_ADDR	equ	0x70000			; MBR real-mode load buffer (below 1 MiB)
KERNEL_BIN_BASE_ADDR	equ	0x100000		; kernel link / run address (GRUB + loader)
KERNEL_COPY_BYTES	equ	KERNEL_SECTOR_COUNT * 512	; bytes copied staging -> link addr
KERNEL_VMA_BASE_LO	equ	0x80000000		; low 32 bits of kernel link address
KERNEL_VMA		equ	0xffffffff80000000	; canonical high-half kernel virtual base

; Offset from LOADER_BASE_ADDR to loader_start in loader.asm (after GDT, buffers).
; MBR jumps here after loading the loader; must match loader layout.
LOADER_ENTRY	equ	LOADER_BASE_ADDR + 0x11c	; physical entry jumped to by MBR

; --- Page tables and boot_info in low memory ---
PAGE_TABLE_BASE		equ	0x90000			; physical base of 5 x 4 KiB page-table pages
BOOT_INFO_ADDR		equ	0x800			; struct boot_info written by loader for kmain

; --- Kernel stack in long mode (canonical high half) ---
STACK_TOP_LO		equ	0x8009f000		; low 32 bits of initial RSP
STACK_TOP_HI		equ	0xffffffff		; high 32 bits of initial RSP

; --- Page-table entry flag bits (x86-64 PTE/PDE) ---
PT_NULL			equ	0			; null pointer value for unused entries

PAGE_PRESENT		equ	1 << 0			; P: page mapped and present
PAGE_WRITE		equ	1 << 1			; R/W: writable by supervisor
PAGE_SIZE_2M		equ	1 << 7			; PS: 2 MiB huge page (PD level)

%include "gdt.inc"					; segment descriptor constants and selectors
