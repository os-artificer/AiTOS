; Copyright 2026 AiTOS authors.
; SPDX-License-Identifier: Apache-2.0
;
; GDT segment-descriptor flag constants and segment selectors for the boot
; loader. Used when building 32-bit protected-mode and 64-bit long-mode
; descriptors in loader.asm.

; --- Descriptor attribute bits (byte 6–7 of an 8-byte segment descriptor) ---
DESC_G_4K		equ	1 << 23				; G=1: limit scaled by 4 KiB granularity
DESC_D_32		equ	1 << 22				; D/B=1: 32-bit default operand size (code/data)
DESC_L_64		equ	1 << 21				; L=1: 64-bit code segment (long mode only)
DESC_AVL		equ	0				; AVL bit unused by hardware, reserved for software
DESC_LIMIT_HI		equ	0x000F0000			; upper 4 bits of 20-bit segment limit
DESC_P			equ	1 << 15				; P=1: segment present in memory
DESC_DPL_0		equ	0 << 13				; DPL 0: ring-0 (most privileged)
DESC_S_CODE		equ	1 << 12				; S=1: non-system (code or data) segment
DESC_S_DATA		equ	1 << 12				; S=1: non-system (code or data) segment
DESC_TYPE_CODE		equ	0x8 << 8			; type 1000: execute/read code segment
DESC_TYPE_DATA		equ	0x2 << 8			; type 0010: expand-up read/write data

; --- Pre-built high dwords for common descriptor types ---
; Limit=0xFFFFF (4 GiB with G=1), base=0, DPL=0, present.
DESC_CODE32_HIGH	equ	(0x00 << 24) | DESC_G_4K | DESC_D_32 | DESC_AVL | \
				DESC_LIMIT_HI | DESC_P | DESC_DPL_0 | DESC_S_CODE | DESC_TYPE_CODE
DESC_DATA_HIGH		equ	(0x00 << 24) | DESC_G_4K | DESC_D_32 | DESC_AVL | \
				DESC_LIMIT_HI | DESC_P | DESC_DPL_0 | DESC_S_DATA | DESC_TYPE_DATA
DESC_CODE64_HIGH	equ	(0x00 << 24) | DESC_G_4K | DESC_L_64 | DESC_AVL | \
				DESC_LIMIT_HI | DESC_P | DESC_DPL_0 | DESC_S_CODE | DESC_TYPE_CODE

; --- Segment selector encoding (index << 3 | TI | RPL) ---
RPL0			equ	0				; requested privilege level 0
TI_GDT			equ	0				; TI=0: selector refers to the GDT

SELECTOR_CODE32		equ	(1 << 3) + TI_GDT + RPL0	; GDT entry 1: 32-bit code
SELECTOR_DATA		equ	(2 << 3) + TI_GDT + RPL0	; GDT entry 2: 32-bit data
SELECTOR_CODE64		equ	(3 << 3) + TI_GDT + RPL0	; GDT entry 3: 64-bit code
