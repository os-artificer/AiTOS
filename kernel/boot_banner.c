/* SPDX-License-Identifier: Apache-2.0 */
#include <aitos/init.h>

#include <aitos/printk.h>

void boot_show_banner(void)
{
	printk("========================================\n");
	printk("  AiTOS x86-64  version 0.1.0-mvp\n");
	printk("  Apache-2.0    https://github.com/\n");
	printk("========================================\n");
	printk("\n");
}
