/* SPDX-License-Identifier: Apache-2.0 */
#include <aitos/sched.h>

#include <aitos/console.h>

int __init sched_init(void)
{
	console_puts("sched: stub layer ready\n");
	return 0;
}
