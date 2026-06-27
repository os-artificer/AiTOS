/* Copyright 2026 AiTOS authors.
 * SPDX-License-Identifier: Apache-2.0
 *
 * GNU ld link script for the x86-64 kernel (Makefile: ld -T, then objcopy -O binary).
 *
 * Pipeline: .o files -> kernel.elf -> kernel.bin -> hd60M.img sector 6 -> boot at 0x70000.
 *
 * Consumers:
 *   - Makefile LDFLAGS / objcopy
 *   - arch/x86_64/kernel.asm: ENTRY _start, __bss_start, __bss_end
 *   - arch/x86_64/boot/boot.inc: KERNEL_BIN_BASE_ADDR must match KERNEL_PHYS
 *
 * Layout: .text/.rodata -> RX PT_LOAD; .data/.bss -> RW PT_LOAD.
 *         kernel.bin holds loaded segments only; BSS is NOBITS and zeroed in _start.
 *
 * Maintenance rules:
 *   1. Keep KERNEL_PHYS in sync with boot.inc KERNEL_BIN_BASE_ADDR.
 *   2. If kernel.bin grows, check boot.inc KERNEL_SECTOR_COUNT and Makefile dd count=.
 *   3. With PHDRS defined, every loadable input section needs :text or :data.
 *   4. New __section__("...") in C/asm needs a matching *(...) line (see compiler.h, init.h).
 *   5. Linker symbols used from asm/C must be defined here explicitly.
 *   6. After edits: readelf -l/-S, make, confirm boot still works.
 *
 * When to update:
 *   - Kernel load address (0x70000): KERNEL_PHYS, boot.inc, loader/MBR.
 *   - High-half link (future): KERNEL_VMA != KERNEL_PHYS; keep AT() formula; update loader paging.
 *   - New custom sections, linker symbols, or PT_LOAD / orphan-section linker warnings.
 *   - Enable .initcall.* or drop more metadata: add *(...) or /DISCARD/ entries.
 *
 * Note: boot.inc defines future high-half KERNEL_VMA (0xffffffff80000000); this script still
 *       uses identity map (VMA == PHYS). ENTRY must match kernel.asm global _start.
 */

ENTRY(_start)

/* Link/run address (VMA). Today identical to physical load address. */
KERNEL_VMA = 0x70000;
KERNEL_PHYS = 0x70000;	/* Must match boot.inc KERNEL_BIN_BASE_ADDR */

PHDRS
{
	text PT_LOAD FLAGS(5);	/* PF_R | PF_X — separate RX from RW (no RWX segment) */
	data PT_LOAD FLAGS(6);	/* PF_R | PF_W */
}

SECTIONS
{
	. = KERNEL_VMA;

	.text : AT(KERNEL_PHYS) {
		*(.head.text)		/* _start (kernel.asm) */
		*(.text .text.*)
		*(.init.text)		/* __init in include/aitos/compiler.h */
	} :text

	/* LMA = KERNEL_PHYS + (VMA offset from link base); same for .rodata/.data/.bss below */
	.rodata : AT(KERNEL_PHYS + (ADDR(.rodata) - KERNEL_VMA)) {
		*(.rodata .rodata.*)
	} :text

	.data : AT(KERNEL_PHYS + (ADDR(.data) - KERNEL_VMA)) {
		*(.data .data.*)
		*(.init.data)		/* __initdata in include/aitos/compiler.h */
	} :data

	.bss : AT(KERNEL_PHYS + (ADDR(.bss) - KERNEL_VMA)) {
		__bss_start = .;	/* Zeroed in _start before kmain */
		*(.bss .bss.* COMMON)
		__bss_end = .;
	} :data

	/DISCARD/ : {			/* Not needed in bare-metal flat image */
		*(.note.*)
		*(.eh_frame)
	}
}
