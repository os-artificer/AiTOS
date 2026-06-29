/* SPDX-License-Identifier: Apache-2.0 */
#include <aitos/shell.h>
#include <aitos/console.h>
#include <aitos/boot.h>

#include <aitos/types.h>

void shell_print_help(void)
{
	if (boot_is_multiboot2()) {
		console_puts("Available commands:\n");
		console_puts("  help - show available commands\n");
		console_puts("  clear - clear the screen\n");
		console_puts("  version - show AiTOS version\n");
		console_puts("  echo - echo arguments\n");
		console_puts("  uname - print system information\n");
		console_puts("  exit - leave shell and stop QEMU\n");
		console_puts("  logout - alias of exit\n");
		return;
	}

	console_puts("Available commands:\n"
		     "  help - show available commands\n"
		     "  clear - clear the screen\n"
		     "  version - show AiTOS version\n"
		     "  echo - echo arguments\n"
		     "  uname - print system information\n"
		     "  exit - leave shell and stop QEMU\n"
		     "  logout - alias of exit\n");
}

int shell_cmd_help(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	shell_print_help();
	return 0;
}

int shell_cmd_clear(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	console_clear();
	return 0;
}

int shell_cmd_version(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	console_puts("AiTOS 0.1.0-mvp (x86-64)\n");
	return 0;
}

int shell_cmd_echo(int argc, char **argv)
{
	int i;

	for (i = 1; i < argc; i++) {
		if (i > 1)
			console_puts(" ");
		console_puts(argv[i]);
	}
	console_puts("\n");
	return 0;
}

int shell_cmd_uname(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	console_puts("AiTOS\n");
	return 0;
}

int shell_cmd_exit(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	return SHELL_EXIT;
}
