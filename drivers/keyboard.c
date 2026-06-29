/* SPDX-License-Identifier: Apache-2.0 */
#include <aitos/keyboard.h>
#include <aitos/io.h>
#include <aitos/irq.h>
#include <aitos/boot.h>

#include <aitos/types.h>

#define KBD_BUF_PORT	0x60
#define COM1_BASE	0x3f8
#define COM1_DATA	(COM1_BASE + 0)
#define COM1_IER	(COM1_BASE + 1)
#define COM1_FCR	(COM1_BASE + 2)
#define COM1_LCR	(COM1_BASE + 3)
#define COM1_MCR	(COM1_BASE + 4)
#define COM1_LSR	(COM1_BASE + 5)
#define KBD_BUF_SIZE	128

static char kbd_ring[KBD_BUF_SIZE];
static unsigned int kbd_head;
static unsigned int kbd_tail;

static char com1_ring[KBD_BUF_SIZE];
static unsigned int com1_head;
static unsigned int com1_tail;

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

	if (!ch || next == kbd_tail)
		return;
	kbd_ring[kbd_head] = ch;
	kbd_head = next;
}

static void com1_push(unsigned char ch)
{
	unsigned int next = (com1_head + 1) % KBD_BUF_SIZE;

	if (!ch || next == com1_tail)
		return;
	com1_ring[com1_head] = (char)ch;
	com1_head = next;
}

static int com1_pop(unsigned char *out)
{
	if (com1_head == com1_tail)
		return 0;
	*out = (unsigned char)com1_ring[com1_tail];
	com1_tail = (com1_tail + 1) % KBD_BUF_SIZE;
	return 1;
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

static void com1_drain_rx(void)
{
	int limit = 256;

	while (limit-- > 0) {
		u8 lsr = inb(COM1_LSR);

		if (lsr == 0xff || !(lsr & 1))
			break;
		(void)inb(COM1_DATA);
	}
}

static void com1_init(void)
{
	/*
	 * GRUB leaves COM1 configured for serial console (115200 8N1).
	 * Re-init so RX/TX work reliably after the handoff.
	 */
	outb(0x00, COM1_IER);
	outb(0x80, COM1_LCR);
	outb(0x01, COM1_DATA);
	outb(0x00, COM1_IER);
	outb(0x03, COM1_LCR);
	outb(0x00, COM1_FCR);
	outb(0x03, COM1_MCR);
	com1_drain_rx();
}

static int com1_char_valid(unsigned char c)
{
	if (boot_is_multiboot2()) {
		return (c >= 32 && c <= 126) || c == '\n' || c == '\r' ||
		       c == '\b' || c == '\t' || c == 3;
	}
	return (c >= 32 && c <= 126) || c == '\n' || c == '\r' ||
	       c == '\b' || c == '\t' || c == 3 || c == 4;
}

static int com1_try_read(unsigned char *out)
{
	u8 lsr = inb(COM1_LSR);
	unsigned char c;

	if (lsr == 0xff)
		return 0;
	if (lsr & 0x02) {
		(void)inb(COM1_DATA);
		return 0;
	}
	if (!(lsr & 0x01))
		return 0;

	c = (unsigned char)inb(COM1_DATA);
	if (!com1_char_valid(c))
		return 0;

	*out = c;
	return 1;
}

static int com1_read_direct(unsigned char *out)
{
	u8 lsr = inb(COM1_LSR);
	unsigned char c;

	if (lsr == 0xff)
		return 0;
	if (lsr & 0x02) {
		(void)inb(COM1_DATA);
		return 0;
	}
	if (!(lsr & 0x01))
		return 0;

	c = (unsigned char)inb(COM1_DATA);
	if (!com1_char_valid(c))
		return 0;

	*out = c;
	return 1;
}

static void com1_poll(void)
{
	unsigned char c;

	while (com1_try_read(&c))
		com1_push(c);
}

int __init keyboard_init(void)
{
	if (boot_is_multiboot2())
		com1_init();
	else
		register_irq_handler(0x21, kbd_irq);
	return 0;
}

void keyboard_reinit_com1(void)
{
	if (boot_is_multiboot2())
		com1_init();
}

void keyboard_drain_com1(void)
{
	if (boot_is_multiboot2()) {
		com1_drain_rx();
		return;
	}
	com1_head = com1_tail = 0;
}

int keyboard_read_char(void)
{
	unsigned char c;

	if (boot_is_multiboot2()) {
		for (;;) {
			if (com1_read_direct(&c)) {
				if (c == '\r')
					c = '\n';
				return (int)c;
			}
			asm volatile("pause");
		}
	}

	for (;;) {
		if (kbd_head != kbd_tail) {
			char ch = kbd_ring[kbd_tail];

			kbd_tail = (kbd_tail + 1) % KBD_BUF_SIZE;
			return (unsigned char)ch;
		}
		com1_poll();
		if (com1_pop(&c)) {
			if (c == '\r')
				c = '\n';
			return (int)c;
		}
		asm volatile("pause");
	}
}
