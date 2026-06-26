/* SPDX-License-Identifier: Apache-2.0 */
#ifndef _AITOS_INIT_H
#define _AITOS_INIT_H

#include <aitos/compiler.h>

typedef int (*initcall_t)(void);

#define __define_initcall(fn, id)					\
	static initcall_t __initcall_##fn##id __used		\
		__attribute__((__section__(".initcall.init"))) = fn

#define early_initcall(fn)	__define_initcall(fn, 0)

void do_initcalls(void);
void __init aitos_init(void);
void boot_show_banner(void);

#endif /* _AITOS_INIT_H */
