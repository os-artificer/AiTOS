/* SPDX-License-Identifier: Apache-2.0 */
#include <aitos/shell.h>
#include <aitos/console.h>

#include <aitos/printk.h>
#include <aitos/types.h>

#include <aitos/string.h>

static int cmd_help(int argc, char **argv);
static int cmd_clear(int argc, char **argv);
static int cmd_version(int argc, char **argv);
static int cmd_echo(int argc, char **argv);
static int cmd_uname(int argc, char **argv);
static int cmd_exit(int argc, char **argv);

static const struct shell_cmd shell_cmds[] = {
	{ "help",    cmd_help,    "show available commands" },
	{ "clear",   cmd_clear,   "clear the screen" },
	{ "version", cmd_version, "show AiTOS version" },
	{ "echo",    cmd_echo,    "echo arguments" },
	{ "uname",   cmd_uname,   "print system information" },
	{ "exit",    cmd_exit,    "leave shell and stop QEMU" },
	{ "logout",  cmd_exit,    "alias of exit" },
	{ NULL, NULL, NULL },
};

int shell_dispatch(int argc, char **argv)
{
	const struct shell_cmd *cmd;
	int i;

	if (argc < 1 || !argv[0] || !argv[0][0])
		return 0;

	for (i = 0; shell_cmds[i].name; i++) {
		cmd = &shell_cmds[i];
		if (!strcmp(argv[0], cmd->name))
			return cmd->handler(argc, argv);
	}

	printk("aitos-sh: %s: command not found\n", argv[0]);
	return -1;
}

void shell_print_help(void)
{
	const struct shell_cmd *cmd;
	int i;

	printk("Available commands:\n");
	for (i = 0; shell_cmds[i].name; i++) {
		cmd = &shell_cmds[i];
		printk("  %s - %s\n", cmd->name, cmd->help);
	}
}

static int cmd_help(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	shell_print_help();
	return 0;
}

static int cmd_clear(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	console_clear();
	return 0;
}

static int cmd_version(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	printk("AiTOS 0.1.0-mvp (x86-64)\n");
	return 0;
}

static int cmd_echo(int argc, char **argv)
{
	int i;

	for (i = 1; i < argc; i++) {
		if (i > 1)
			printk(" ");
		printk("%s", argv[i]);
	}
	printk("\n");
	return 0;
}

static int cmd_uname(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	printk("AiTOS\n");
	return 0;
}

static int cmd_exit(int argc, char **argv)
{
	(void)argc;
	(void)argv;
	return SHELL_EXIT;
}
