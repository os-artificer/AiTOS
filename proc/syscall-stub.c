/* SPDX-License-Identifier: Apache-2.0 */
#include <aitos/syscall.h>

#include <aitos/printk.h>

int __init syscall_init_stubs(void)
{
	pr_info("syscall: stub layer ready\n");
	return 0;
}
