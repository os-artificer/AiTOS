/* SPDX-License-Identifier: Apache-2.0 */
#include <aitos/init.h>

#include <aitos/console.h>

void boot_show_banner(void)
{
	console_puts("========================================\n");
	console_puts("  AiTOS x86-64  version 0.1.0-mvp\n");
	console_puts("  Apache-2.0    https://github.com/\n");
	console_puts("========================================\n");
	console_puts("\n");
}
