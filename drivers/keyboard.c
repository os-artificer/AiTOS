/* SPDX-License-Identifier: Apache-2.0 */
#include <aitos/keyboard.h>
#include <aitos/io.h>
#include <aitos/irq.h>

#include <aitos/types.h>

#define KBD_BUF_PORT	0x60
#define COM1_DATA	0x3f8
#define COM1_LSR	0x3fd
#define KBD_BUF_SIZE	128

static char kbd_ring[KBD_BUF_SIZE];
static unsigned int kbd_head;
static unsigned int kbd_tail;

static u8 shift_down;
static u8 ext_scancode;

static const char keymap[128] = {
	0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
	'\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\r',
	0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
	0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
	'*', 0, ' '
};

static const char keymap_shift[128] = {
	0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
	'\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\r',
	0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
	0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
	'*', 0, ' '
};

static void kbd_push(char ch)
{
	unsigned int next = (kbd_head + 1) % KBD_BUF_SIZE;

	if (next == kbd_tail)
		return;
	kbd_ring[kbd_head] = ch;
	kbd_head = next;
}

static void kbd_irq(void)
{
	u8 sc = inb(KBD_BUF_PORT);

	if (!sc) {
		ext_scancode = 0;
		return;
	}
	if (sc == 0xe0) {
		ext_scancode = 1;
		return;
	}
	if (sc == 0x2a || sc == 0x36) {
		shift_down = 1;
		return;
	}
	if (sc == 0xaa || sc == 0xb6) {
		shift_down = 0;
		return;
	}
	if (sc & 0x80)
		return;
	if (sc >= 128)
		return;

	if (shift_down)
		kbd_push(keymap_shift[sc]);
	else
		kbd_push(keymap[sc]);
	ext_scancode = 0;
}

int __init keyboard_init(void)
{
	register_irq_handler(0x21, kbd_irq);
	return 0;
}

int keyboard_read_char(void)
{
	for (;;) {
		if (kbd_head != kbd_tail) {
			char c = kbd_ring[kbd_tail];

			kbd_tail = (kbd_tail + 1) % KBD_BUF_SIZE;
			return (unsigned char)c;
		}
		/*
		 * Headless QEMU (-nographic) delivers terminal stdin to COM1.
		 * Guard against a missing UART: an absent port reads 0xFF on
		 * every line, which would otherwise inject garbage characters.
		 */
		u8 lsr = inb(COM1_LSR);

		if (lsr != 0xff && (lsr & 1))
			return (unsigned char)inb(COM1_DATA);
		asm volatile("pause");
	}
}
