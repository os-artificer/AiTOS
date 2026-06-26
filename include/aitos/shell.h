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

#endif /* _AITOS_SHELL_H */
