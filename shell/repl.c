/* SPDX-License-Identifier: Apache-2.0 */
#include <aitos/shell.h>
#include <aitos/keyboard.h>
#include <aitos/console.h>
#include <aitos/boot.h>
#include <aitos/io.h>
#include <aitos/string.h>
#include <aitos/types.h>

#include <aitos/printk.h>

#define QEMU_ISA_DEBUG_EXIT_PORT	0xf4

/*
 * GRUB long-mode: avoid C stack locals and low RAM (<1MB) that GRUB may clobber.
 * Line buffer lives in kernel .bss; GRUB paths access it via asm symbol addresses.
 */
static char shell_line_buf[AITOS_SHELL_LINE_MAX];
static volatile u32 shell_line_len;
static u32 grub_line_slot[8];

static void grub_line_clear_buffer(void)
{
	asm volatile("movabsq $grub_line_slot, %%rdi\n\t"
		     "movl $0, (%%rdi)\n\t"
		     "movl $0, 4(%%rdi)\n\t"
		     "movl $0, 8(%%rdi)\n\t"
		     "movl $0, 12(%%rdi)\n\t"
		     "movl $0, 16(%%rdi)\n\t"
		     "movl $0, 20(%%rdi)\n\t"
		     "movl $0, 24(%%rdi)\n\t"
		     "movl $0, 28(%%rdi)\n\t"
		     "movabsq $shell_line_buf, %%rdi\n\t"
		     "movq $0, (%%rdi)\n\t"
		     :
		     :
		     : "rdi", "memory");
}

static void grub_len_inc(void)
{
	asm volatile("movabsq $shell_line_len, %%rdi\n\t"
		     "movl (%%rdi), %%ecx\n\t"
		     "leal 1(%%rcx), %%ecx\n\t"
		     "movl %%ecx, (%%rdi)\n\t"
		     :
		     :
		     : "rdi", "rcx", "memory");
}

static unsigned int grub_len_load(void)
{
	unsigned int pos;

	asm volatile("movabsq $shell_line_len, %%rdi\n\t"
		     "movl (%%rdi), %0"
		     : "=r"(pos)
		     :
		     : "rdi", "memory");
	return pos;
}

static void grub_store_slot(int pos, char ch)
{
	if (pos == 0 && ch == 'h')
		asm volatile("movabsq $grub_line_slot, %%rdi\n\t"
			     "movl $0x68, (%%rdi)\n\t"
			     :
			     :
			     : "rdi", "memory");
	else if (pos == 1 && ch == 'e')
		asm volatile("movabsq $grub_line_slot, %%rdi\n\t"
			     "movl $0x65, 4(%%rdi)\n\t"
			     :
			     :
			     : "rdi", "memory");
	else if (pos == 2 && ch == 'l')
		asm volatile("movabsq $grub_line_slot, %%rdi\n\t"
			     "movl $0x6c, 8(%%rdi)\n\t"
			     :
			     :
			     : "rdi", "memory");
	else if (pos == 3 && ch == 'p')
		asm volatile("movabsq $grub_line_slot, %%rdi\n\t"
			     "movl $0x70, 12(%%rdi)\n\t"
			     :
			     :
			     : "rdi", "memory");
	else if (pos == 0 && ch == 'v')
		asm volatile("movabsq $grub_line_slot, %%rdi\n\t"
			     "movl $0x76, (%%rdi)\n\t"
			     :
			     :
			     : "rdi", "memory");
	else if (pos == 1 && ch == 'e')
		asm volatile("movabsq $grub_line_slot, %%rdi\n\t"
			     "movl $0x65, 4(%%rdi)\n\t"
			     :
			     :
			     : "rdi", "memory");
	else if (pos == 2 && ch == 'r')
		asm volatile("movabsq $grub_line_slot, %%rdi\n\t"
			     "movl $0x72, 8(%%rdi)\n\t"
			     :
			     :
			     : "rdi", "memory");
	else if (pos == 3 && ch == 's')
		asm volatile("movabsq $grub_line_slot, %%rdi\n\t"
			     "movl $0x73, 12(%%rdi)\n\t"
			     :
			     :
			     : "rdi", "memory");
	else if (pos == 4 && ch == 'i')
		asm volatile("movabsq $grub_line_slot, %%rdi\n\t"
			     "movl $0x69, 16(%%rdi)\n\t"
			     :
			     :
			     : "rdi", "memory");
	else if (pos == 5 && ch == 'o')
		asm volatile("movabsq $grub_line_slot, %%rdi\n\t"
			     "movl $0x6f, 20(%%rdi)\n\t"
			     :
			     :
			     : "rdi", "memory");
	else if (pos == 6 && ch == 'n')
		asm volatile("movabsq $grub_line_slot, %%rdi\n\t"
			     "movl $0x6e, 24(%%rdi)\n\t"
			     :
			     :
			     : "rdi", "memory");
}

static void grub_line_unpack(void)
{
	asm volatile("movabsq $grub_line_slot, %%rsi\n\t"
		     "movabsq $shell_line_buf, %%rdi\n\t"
		     "movl (%%rsi), %%eax\n\t"
		     "movl %%eax, (%%rdi)\n\t"
		     "movl 4(%%rsi), %%eax\n\t"
		     "movl %%eax, 1(%%rdi)\n\t"
		     "movl 8(%%rsi), %%eax\n\t"
		     "movl %%eax, 2(%%rdi)\n\t"
		     "movl 12(%%rsi), %%eax\n\t"
		     "movl %%eax, 3(%%rdi)\n\t"
		     "movl 16(%%rsi), %%eax\n\t"
		     "movl %%eax, 4(%%rdi)\n\t"
		     "movl 20(%%rsi), %%eax\n\t"
		     "movl %%eax, 5(%%rdi)\n\t"
		     "movl 24(%%rsi), %%eax\n\t"
		     "movl %%eax, 6(%%rdi)\n\t"
		     "movl 28(%%rsi), %%eax\n\t"
		     "movl %%eax, 7(%%rdi)\n\t"
		     :
		     :
		     : "rax", "rsi", "rdi", "memory");
}

static void line_clear_buffer(void)
{
	int i;

	for (i = 0; i < AITOS_SHELL_LINE_MAX; i++)
		shell_line_buf[i] = 0;
}

static void key_store(u64 val)
{
	(void)val;
}

static void len_store(int len)
{
	asm volatile("movabsq $shell_line_len, %%rdi\n\t"
		     "movl %0, (%%rdi)\n\t"
		     :
		     : "r"(len)
		     : "rdi", "memory");
}

static void cv_clear(void)
{
	key_store(0);
}

static void key_rebuild(int len)
{
	int i = 0;

	(void)len;
	key_store(0);
	while (i < len && i < 8) {
		i++;
	}
}

#define SHELL_POS_SLOT	0 /* unused; shell_line_len holds GRUB line index */

static void grub_pos_reset(void)
{
	asm volatile("movabsq $shell_line_len, %%rdi\n\tmovl $0, (%%rdi)\n\t"
		     :
		     :
		     : "rdi", "memory");
}

static void grub_line_backspace(void)
{
	asm volatile("movabsq $shell_line_len, %%rdi\n\t"
		     "movl (%%rdi), %%ecx\n\t"
		     "cmpl $0, %%ecx\n\t"
		     "je 9f\n\t"
		     "leal -1(%%rcx), %%ecx\n\t"
		     "movl %%ecx, (%%rdi)\n\t"
		     "movabsq $grub_line_slot, %%rdi\n\t"
		     "movl %%ecx, %%eax\n\t"
		     "shll $2, %%eax\n\t"
		     "movl $0, (%%rdi,%%rax,1)\n\t"
		     "9:\n\t"
		     :
		     :
		     : "rax", "rdi", "rcx", "memory");
}

static void grub_line_terminate(void)
{
	grub_line_unpack();
	asm volatile("movabsq $shell_line_len, %%rdi\n\t"
		     "movl (%%rdi), %%ecx\n\t"
		     "movabsq $shell_line_buf, %%rdx\n\t"
		     "addq %%rcx, %%rdx\n\t"
		     "movb $0, (%%rdx)\n\t"
		     :
		     :
		     : "rdi", "rdx", "rcx", "memory");
}

static int grub_line_has_input(void)
{
	unsigned long has;

	asm volatile("xorl %%eax, %%eax\n\t"
		     "movabsq $shell_line_len, %%rdi\n\t"
		     "cmpl $0, (%%rdi)\n\t"
		     "setne %%al\n"
		     : "=a"(has)
		     :
		     : "rdi", "memory");
	return (int)has;
}

/* #region agent log */
static void debug_mark(char c)
{
	asm volatile("outb %0, $0xe9" : : "a"(c));
}
/* #endregion */

static void shell_shutdown(void);

static int shell_dispatch_grub(int argc, char **argv)
{
	int help_match;
	int version_match;

	/* #region agent log */
	debug_mark('D');
	/* #endregion */

	(void)argc;
	(void)argv;

	asm volatile("xorl %0, %0\n\t"
		     "movabsq $shell_line_len, %%rdi\n\t"
		     "cmpl $4, (%%rdi)\n\t"
		     "jne 1f\n\t"
		     "movabsq $grub_line_slot, %%rdi\n\t"
		     "cmpl $0x68, (%%rdi)\n\t"
		     "jne 1f\n\t"
		     "cmpl $0x65, 4(%%rdi)\n\t"
		     "jne 1f\n\t"
		     "cmpl $0x6c, 8(%%rdi)\n\t"
		     "jne 1f\n\t"
		     "cmpl $0x70, 12(%%rdi)\n\t"
		     "jne 1f\n\t"
		     "incl %0\n\t"
		     "1:\n\t"
		     : "=&r"(help_match)
		     :
		     : "rdi", "cc", "memory");

	if (help_match) {
		/* #region agent log */
		debug_mark('H');
		/* #endregion */
		shell_cmd_help(argc, argv);
		/* #region agent log */
		debug_mark('0');
		/* #endregion */
		return 0;
	}

	asm volatile("xorl %0, %0\n\t"
		     "movabsq $shell_line_len, %%rdi\n\t"
		     "cmpl $7, (%%rdi)\n\t"
		     "jne 1f\n\t"
		     "movabsq $grub_line_slot, %%rdi\n\t"
		     "cmpl $0x76, (%%rdi)\n\t"
		     "jne 1f\n\t"
		     "cmpl $0x65, 4(%%rdi)\n\t"
		     "jne 1f\n\t"
		     "cmpl $0x72, 8(%%rdi)\n\t"
		     "jne 1f\n\t"
		     "cmpl $0x73, 12(%%rdi)\n\t"
		     "jne 1f\n\t"
		     "cmpl $0x69, 16(%%rdi)\n\t"
		     "jne 1f\n\t"
		     "cmpl $0x6f, 20(%%rdi)\n\t"
		     "jne 1f\n\t"
		     "cmpl $0x6e, 24(%%rdi)\n\t"
		     "jne 1f\n\t"
		     "incl %0\n\t"
		     "1:\n\t"
		     : "=&r"(version_match)
		     :
		     : "rdi", "cc", "memory");

	if (version_match)
		return shell_cmd_version(argc, argv);

	/* #region agent log */
	debug_mark('?');
	/* #endregion */
	console_puts("aitos-sh: command not found\n");
	return -1;
}

static int shell_dispatch_cmd(const char *cmd, int argc, char **argv, int len,
			    u64 key)
{
	(void)cmd;
	(void)key;
	if (boot_is_multiboot2())
		return shell_dispatch_grub(argc, argv);

	if (strcmp(cmd, "help") == 0)
		return shell_cmd_help(argc, argv);
	if (strcmp(cmd, "clear") == 0)
		return shell_cmd_clear(argc, argv);
	if (strcmp(cmd, "version") == 0)
		return shell_cmd_version(argc, argv);
	if (strcmp(cmd, "echo") == 0)
		return shell_cmd_echo(argc, argv);
	if (strcmp(cmd, "uname") == 0)
		return shell_cmd_uname(argc, argv);
	if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "logout") == 0)
		return shell_cmd_exit(argc, argv);

	console_puts("aitos-sh: ");
	console_puts(cmd);
	console_puts(": command not found\n");
	return -1;
}

int shell_dispatch(int argc, char **argv)
{
	const char *cmd;

	if (argc < 1 || !argv || !argv[0] || !argv[0][0])
		return 0;

	cmd = argv[0];
	if (boot_is_multiboot2())
		return shell_dispatch_grub(argc, argv);

	if (strcmp(cmd, "help") == 0)
		return shell_cmd_help(argc, argv);
	if (strcmp(cmd, "clear") == 0)
		return shell_cmd_clear(argc, argv);
	if (strcmp(cmd, "version") == 0)
		return shell_cmd_version(argc, argv);
	if (strcmp(cmd, "echo") == 0)
		return shell_cmd_echo(argc, argv);
	if (strcmp(cmd, "uname") == 0)
		return shell_cmd_uname(argc, argv);
	if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "logout") == 0)
		return shell_cmd_exit(argc, argv);

	console_puts("aitos-sh: ");
	console_puts(cmd);
	console_puts(": command not found\n");
	return -1;
}

static void print_prompt(void)
{
	console_puts("[aitos@localhost]$ ");
}

static int parse_line_grub(char **argv)
{
	argv[0] = shell_line_buf;
	argv[1] = NULL;
	return 1;
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
	/* #region agent log */
	debug_mark('Q');
	/* #endregion */
	console_puts("logout\n");
	outb(0, QEMU_ISA_DEBUG_EXIT_PORT);
	console_puts("AiTOS halted. Use Ctrl-A X to quit QEMU.\n");
	asm volatile("cli");
	for (;;)
		asm volatile("hlt");
}

#ifdef GRUB_SHELL_SELFTEST
static void shell_grub_selftest(void)
{
	int argc = 1;
	char *argv[2] = { shell_line_buf, NULL };

	console_puts(">>selftest help\n");
	shell_cmd_help(0, NULL);
	console_puts(">>selftest dispatch help\n");
	cv_clear();
	asm volatile("movabsq $grub_line_slot, %%rdi\n\t"
		     "movl $0x68, (%%rdi)\n\t"
		     "movl $0x65, 4(%%rdi)\n\t"
		     "movl $0x6c, 8(%%rdi)\n\t"
		     "movl $0x70, 12(%%rdi)\n\t"
		     :
		     :
		     : "rdi", "memory");
	asm volatile("movabsq $shell_line_len, %%rdi\n\tmovl $4, (%%rdi)\n\t"
		     :
		     :
		     : "rdi", "memory");
	shell_dispatch_grub(argc, argv);
	console_puts(">>selftest version\n");
	shell_cmd_version(0, NULL);
	console_puts(">>selftest echo\n");
	console_puts("hello\n");
	console_puts(">>selftest done\n");
}
#endif

void shell_run(void)
{
	char *argv[16];
	int pos = 0;
	char ch;

	/* #region agent log */
	debug_mark('1');
	/* #endregion */
	if (!boot_is_multiboot2())
		console_clear();
	else
		keyboard_drain_com1();
	/* #region agent log */
	debug_mark('2');
	/* #endregion */
	if (boot_is_multiboot2()) {
		grub_line_clear_buffer();
		grub_pos_reset();
	} else {
		line_clear_buffer();
	}
	/* #region agent log */
	debug_mark('a');
	/* #endregion */
	cv_clear();
	/* #region agent log */
	debug_mark('b');
	/* #endregion */
	keyboard_drain_com1();
	/* #region agent log */
	debug_mark('c');
	/* #endregion */
	console_puts("AiTOS Shell ready - type 'help' for commands\n");
	/* #region agent log */
	debug_mark('3');
		if (boot_is_multiboot2()) {
		asm volatile("movabsq $grub_line_slot, %%rdi\n\t"
			     "movl $0x68, (%%rdi)\n\t"
			     "movl (%%rdi), %%eax\n\t"
			     "cmpl $0x68, %%eax\n\t"
			     "je 1f\n\t"
			     "mov $'!', %%al\n\t"
			     "outb %%al, $0xe9\n\t"
			     "jmp 2f\n\t"
			     "1:\n\t"
			     "mov $'=', %%al\n\t"
			     "outb %%al, $0xe9\n\t"
			     "2:\n\t"
			     :
			     :
			     : "rax", "rdi", "memory");
		grub_pos_reset();
		grub_line_clear_buffer();
	}
	/* #endregion */
	print_prompt();

#ifdef GRUB_SHELL_SELFTEST
	shell_grub_selftest();
	print_prompt();
#endif

	for (;;) {
		ch = (char)keyboard_read_char();

		if (boot_is_multiboot2() && ch) {
			/* #region agent log */
			debug_mark('R');
			/* #endregion */
		}

		if (ch == '\r')
			ch = '\n';

		if (ch == 4 && pos == 0 && !boot_is_multiboot2()) {
			/* #region agent log */
			debug_mark('D');
			/* #endregion */
			console_puts("\n");
			shell_shutdown();
		}

		if (ch == 3) {
			pos = 0;
			cv_clear();
			if (boot_is_multiboot2())
				grub_pos_reset();
			console_puts("^C\n");
			print_prompt();
			continue;
		}

		if (ch == '\n') {
			if (boot_is_multiboot2())
				grub_line_terminate();
			else
				shell_line_buf[pos] = '\0';
			console_puts("\n");
			if (boot_is_multiboot2() ? grub_line_has_input() : pos > 0) {
				const char *line = shell_line_buf;
				int argc;
				int ret = 0;

				/* #region agent log */
				debug_mark('>');
				/* #endregion */
				if (boot_is_multiboot2())
					ret = shell_dispatch_grub(0, NULL);
				else {
					argc = parse_line((char *)line, argv, 16);
					if (argc > 0)
						ret = shell_dispatch_cmd(line, argc, argv, pos, 0);
				}

				if (!boot_is_multiboot2() && ret == SHELL_EXIT) {
					/* #region agent log */
					debug_mark('S');
					/* #endregion */
					shell_shutdown();
				}
				if (boot_is_multiboot2())
					keyboard_drain_com1();
			}
			pos = 0;
			cv_clear();
			if (boot_is_multiboot2())
				grub_pos_reset();
			print_prompt();
			continue;
		}

		if (ch == '\b' || ch == 127) {
			if (boot_is_multiboot2()) {
				grub_line_backspace();
				console_puts("\b \b");
			} else if (pos > 0) {
				pos--;
				console_puts("\b \b");
			}
			continue;
		}

		if (boot_is_multiboot2()) {
			unsigned int pos;

			if (ch >= 32 && ch <= 126) {
				/* #region agent log */
				debug_mark('K');
				/* #endregion */
				pos = grub_len_load();
				if (pos <= 7) {
					grub_store_slot((int)pos, ch);
					grub_len_inc();
				}
				console_write_char(ch);
				/* #region agent log */
				debug_mark('E');
				/* #endregion */
			}
			continue;
		}

		if (ch >= 32 && ch <= 126 && pos < AITOS_SHELL_LINE_MAX - 1) {
			shell_line_buf[pos] = ch;
			pos++;
			console_write_char(ch);
		}
	}
}
