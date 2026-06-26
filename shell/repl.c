/* SPDX-License-Identifier: Apache-2.0 */
#include <aitos/shell.h>
#include <aitos/keyboard.h>
#include <aitos/console.h>
#include <aitos/io.h>
#include <aitos/string.h>

#include <aitos/printk.h>

#define QEMU_ISA_DEBUG_EXIT_PORT	0xf4

static char line_buf[AITOS_SHELL_LINE_MAX];
static char *argv_buf[16];

static void print_prompt(void)
{
	printk("[aitos@localhost]$ ");
}

static int parse_line(char *line, char **argv, int max_arg)
{
	int argc = 0;
	char *p = line;

	while (*p == ' ' || *p == '\t')
		p++;

	while (*p && argc < max_arg - 1) {
		argv[argc++] = p;
		while (*p && *p != ' ' && *p != '\t')
			p++;
		if (!*p)
			break;
		*p++ = '\0';
		while (*p == ' ' || *p == '\t')
			p++;
	}
	argv[argc] = NULL;
	return argc;
}

static void shell_shutdown(void)
{
	printk("logout\n");
	outb(0, QEMU_ISA_DEBUG_EXIT_PORT);
	printk("AiTOS halted. Use Ctrl-A X to quit QEMU.\n");
	asm volatile("cli");
	for (;;)
		asm volatile("hlt");
}

void shell_run(void)
{
	int pos = 0;
	char ch;

	console_clear();
	printk("AiTOS Shell ready - type 'help' for commands\n");
	print_prompt();

	for (;;) {
		ch = (char)keyboard_read_char();

		if (ch == '\r')
			ch = '\n';

		if (ch == 4 && pos == 0) {
			printk("\n");
			shell_shutdown();
		}

		if (ch == 3) {
			pos = 0;
			printk("^C\n");
			print_prompt();
			continue;
		}

		if (ch == '\n') {
			line_buf[pos] = '\0';
			printk("\n");
			if (pos > 0) {
				int argc = parse_line(line_buf, argv_buf, 16);
				int ret = shell_dispatch(argc, argv_buf);

				if (ret == SHELL_EXIT)
					shell_shutdown();
			}
			pos = 0;
			print_prompt();
			continue;
		}

		if (ch == '\b' || ch == 127) {
			if (pos > 0) {
				pos--;
				printk("\b \b");
			}
			continue;
		}

		if (ch >= 32 && ch <= 126 && pos < AITOS_SHELL_LINE_MAX - 1) {
			line_buf[pos++] = ch;
			printk("%c", ch);
		}
	}
}
