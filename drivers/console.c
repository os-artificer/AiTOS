/* SPDX-License-Identifier: Apache-2.0 */
#include <aitos/console.h>

extern void arch_console_putchar(char c);
extern void arch_console_reset_cursor(void);
void __init vga_init_text_mode(void);
void vga_clear_screen(void);

static const struct console_ops vga_console = {
	.name = "vga",
	.write_char = arch_console_putchar,
	.clear = arch_console_reset_cursor,
};

static const struct console_ops *active_console;

void register_console(const struct console_ops *ops)
{
	active_console = ops;
}

void console_write_char(char c)
{
	if (active_console && active_console->write_char)
		active_console->write_char(c);
	else
		arch_console_putchar(c);
}

void console_clear(void)
{
	vga_clear_screen();
	arch_console_reset_cursor();
}

int __init console_init(void)
{
	vga_init_text_mode();
	register_console(&vga_console);
	console_clear();
	return 0;
}
