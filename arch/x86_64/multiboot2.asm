; Copyright 2026 AiTOS authors.
; SPDX-License-Identifier: Apache-2.0
;
; Multiboot2 header — placed after _start in .text (see linker.ld).

section .multiboot
align 8

%define MB2_MAGIC        0xE85250D6
%define MB2_ARCH         0
%define MB2_TAG_INFO     1
%define MB2_TAG_MMAP     6
%define MB2_TAG_END      0

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

    align 8
    dw MB2_TAG_END
    dw 0
    dd 8

header_end:
