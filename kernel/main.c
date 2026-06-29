/* SPDX-License-Identifier: Apache-2.0 */
#include <aitos/init.h>
#include <aitos/console.h>
#include <aitos/irq.h>
#include <aitos/keyboard.h>
#include <aitos/mm.h>
#include <aitos/sched.h>
#include <aitos/syscall.h>
#include <aitos/fs.h>
#include <aitos/shell.h>

#include <aitos/printk.h>

/* #region agent log */
static void debug_mark(char c)
{
	asm volatile("outb %0, $0xe9" : : "a"(c));
}
/* #endregion */

static void __init init_console_irq(void)
{
	console_init();
	irq_init();
}

static void __init init_subsystems(void)
{
	keyboard_init();
	mm_init_bootstrap();
	sched_init();
	syscall_init_stubs();
	fs_init_stub();
	console_puts("aitos: initialization complete\n");
}

void kmain(void)
{
	init_console_irq();
	boot_show_banner();
	init_subsystems();
	/* #region agent log */
	debug_mark('I');
	/* #endregion */
	intr_enable();
	/* #region agent log */
	debug_mark('E');
	/* #endregion */
	shell_run();
}
