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

LOADER_BASE_ADDR	equ	0x900
LOADER_START_SECTOR	equ	0x2

KERNEL_START_SECTOR	equ	0x6
KERNEL_SECTOR_COUNT	equ	40
KERNEL_BIN_BASE_ADDR	equ	0x70000
KERNEL_VMA_BASE_LO	equ	0x80000000
KERNEL_VMA		equ	0xffffffff80000000
LOADER_ENTRY	equ	LOADER_BASE_ADDR + 0x11c

PAGE_TABLE_BASE		equ	0x90000
BOOT_INFO_ADDR		equ	0x800

STACK_TOP_LO		equ	0x8009f000
STACK_TOP_HI		equ	0xffffffff

PT_NULL			equ	0

PAGE_PRESENT		equ	1 << 0
PAGE_WRITE		equ	1 << 1
PAGE_SIZE_2M		equ	1 << 7

%include "gdt.inc"
