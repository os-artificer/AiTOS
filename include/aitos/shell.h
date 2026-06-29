/* SPDX-License-Identifier: Apache-2.0 */
#ifndef _AITOS_SHELL_H
#define _AITOS_SHELL_H

#define AITOS_SHELL_LINE_MAX	128
#define SHELL_EXIT		1

struct shell_cmd {
	const char *name;
	int (*handler)(int argc, char **argv);
	const char *help;
};

int shell_dispatch(int argc, char **argv);
void shell_print_help(void);
void shell_run(void);

int shell_cmd_help(int argc, char **argv);
int shell_cmd_clear(int argc, char **argv);
int shell_cmd_version(int argc, char **argv);
int shell_cmd_echo(int argc, char **argv);
int shell_cmd_uname(int argc, char **argv);
int shell_cmd_exit(int argc, char **argv);

#endif /* _AITOS_SHELL_H */
