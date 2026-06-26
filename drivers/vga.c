/* SPDX-License-Identifier: Apache-2.0 */
#include <aitos/io.h>

#include <aitos/compiler.h>
#include <aitos/types.h>

/*
 * Standard VGA 80x25 colour text mode (mode 0x03) register programmer.
 *
 * On this x86_64 path the BIOS text mode set via int 0x10 in real mode does
 * NOT survive into long mode under QEMU's -vga std: writes to 0xB8000 land in
 * memory (read back fine) but the display controller renders nothing. We must
 * therefore reprogram the VGA controller ourselves to a known text mode.
 *
 * Register values are the canonical mode 0x03 set (see VGA hardware docs).
 */

#define VGA_AC_INDEX	0x3C0
#define VGA_AC_WRITE	0x3C0
#define VGA_MISC_WRITE	0x3C2
#define VGA_SEQ_INDEX	0x3C4
#define VGA_SEQ_DATA	0x3C5
#define VGA_GC_INDEX	0x3CE
#define VGA_GC_DATA	0x3CF
#define VGA_CRTC_INDEX	0x3D4
#define VGA_CRTC_DATA	0x3D5
#define VGA_INSTAT_READ	0x3DA

#define VGA_TEXT_BUF	0xB8000UL
#define VGA_COLS	80
#define VGA_ROWS	25
#define VGA_BLANK	0x0F20

static const u8 misc_reg = 0x67;

static const u8 seq_regs[5] = { 0x03, 0x00, 0x03, 0x00, 0x02 };

static const u8 crtc_regs[25] = {
	0x5F, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F,
	0x00, 0x4F, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x00,
	0x9C, 0x8E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3,
	0xFF,
};

static const u8 gc_regs[9] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00, 0xFF,
};

static const u8 ac_regs[21] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x0C, 0x00, 0x0F, 0x08, 0x00,
};

void __init vga_init_text_mode(void)
{
	unsigned int i;

	/* Miscellaneous output (sets I/O address to 0x3Dx, clock, etc). */
	outb(misc_reg, VGA_MISC_WRITE);

	/* Sequencer: keep in synchronous reset while loading. */
	outb(0x00, VGA_SEQ_INDEX);
	outb(0x01, VGA_SEQ_DATA);
	for (i = 0; i < sizeof(seq_regs); i++) {
		outb((u8)i, VGA_SEQ_INDEX);
		outb(seq_regs[i], VGA_SEQ_DATA);
	}
	/* Release sequencer reset. */
	outb(0x00, VGA_SEQ_INDEX);
	outb(0x03, VGA_SEQ_DATA);

	/* Unlock CRTC registers 0-7 (clear protect bit in CRTC[0x11]). */
	outb(0x11, VGA_CRTC_INDEX);
	outb((u8)(inb(VGA_CRTC_DATA) & 0x7F), VGA_CRTC_DATA);

	for (i = 0; i < sizeof(crtc_regs); i++) {
		outb((u8)i, VGA_CRTC_INDEX);
		/* Keep protect bit clear so all writes take effect. */
		if (i == 0x11)
			outb((u8)(crtc_regs[i] & 0x7F), VGA_CRTC_DATA);
		else
			outb(crtc_regs[i], VGA_CRTC_DATA);
	}

	for (i = 0; i < sizeof(gc_regs); i++) {
		outb((u8)i, VGA_GC_INDEX);
		outb(gc_regs[i], VGA_GC_DATA);
	}

	/* Attribute controller: index/data share 0x3C0 via a flip-flop. */
	for (i = 0; i < sizeof(ac_regs); i++) {
		(void)inb(VGA_INSTAT_READ); /* reset flip-flop to index */
		outb((u8)i, VGA_AC_INDEX);
		outb(ac_regs[i], VGA_AC_WRITE);
	}

	/* Set PAS bit (0x20) to re-enable the display output. */
	(void)inb(VGA_INSTAT_READ);
	outb(0x20, VGA_AC_INDEX);
}

void vga_clear_screen(void)
{
	volatile u16 *buf = (volatile u16 *)VGA_TEXT_BUF;
	unsigned int i;

	for (i = 0; i < VGA_COLS * VGA_ROWS; i++)
		buf[i] = VGA_BLANK;
}
