/* SPDX-License-Identifier: Apache-2.0 */
#include <aitos/console.h>
#include <aitos/boot.h>
#include <aitos/io.h>

extern void arch_console_putchar(char c);
extern void arch_console_reset_cursor(void);
extern void debugcon_puts(const char *s);
extern volatile u64 boot_handoff_source;
void __init vga_init_text_mode(void);
void vga_clear_screen(void);

static const struct console_ops vga_console = {
	.name = "vga",
	.write_char = arch_console_putchar,
	.clear = arch_console_reset_cursor,
};

static const struct console_ops *active_console;
int console_vga_enabled;
int printk_use_serial;

static int console_is_serial(void)
{
	if (boot_handoff_source == BOOT_SOURCE_MULTIBOOT2)
		return 1;
	u16 cs;

	asm volatile("movw %%cs, %0" : "=r"(cs));
	return cs == 0x10 || cs == 0x8;
}

static void com1_putchar(char c)
{
	u8 ch = (u8)c;

	while (!(inb(0x3fd) & 0x20))
		;
	asm volatile("outb %0, $0x3f8" : : "a"(ch));
	if (ch == '\n') {
		while (!(inb(0x3fd) & 0x20))
			;
		asm volatile("outb %0, $0x3f8" : : "a"((u8)'\r'));
	}
}

static void debugcon_putchar(char c)
{
	u8 ch = (u8)c;

	asm volatile("outb %0, $0xe9" : : "a"(ch));
	if (ch == '\n') {
		u8 cr = '\r';

		asm volatile("outb %0, $0xe9" : : "a"(cr));
	}
}

void register_console(const struct console_ops *ops)
{
	active_console = ops;
}

void console_write_char(char c)
{
	if (console_is_serial()) {
		debugcon_putchar(c);
		com1_putchar(c);
		return;
	}
	if (active_console && active_console->write_char)
		active_console->write_char(c);
	else
		arch_console_putchar(c);
}

void console_puts_legacy(const char *s)
{
	while (s && *s)
		console_write_char(*s++);
}

void console_clear(void)
{
	vga_clear_screen();
	arch_console_reset_cursor();
}

int __init console_init(void)
{
	int mb2 = console_is_serial();

	printk_use_serial = mb2;

	if (!mb2) {
		vga_init_text_mode();
		console_vga_enabled = 1;
		register_console(&vga_console);
		console_clear();
	}
	return 0;
}
