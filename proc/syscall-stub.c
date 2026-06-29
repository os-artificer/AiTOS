/* SPDX-License-Identifier: Apache-2.0 */
#include <aitos/syscall.h>

#include <aitos/console.h>

int __init syscall_init_stubs(void)
{
	console_puts("syscall: stub layer ready\n");
	return 0;
}
