/* SPDX-License-Identifier: Apache-2.0 */
#ifndef _AITOS_CONSOLE_H
#define _AITOS_CONSOLE_H

#include <aitos/types.h>
#include <aitos/compiler.h>

struct console_ops {
	const char *name;
	void (*write_char)(char c);
	void (*clear)(void);
};

void register_console(const struct console_ops *ops);
void console_write_char(char c);
void console_clear(void);

int __init console_init(void);

#endif /* _AITOS_CONSOLE_H */
