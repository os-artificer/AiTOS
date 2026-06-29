/* SPDX-License-Identifier: Apache-2.0 */
#include <aitos/fs.h>

#include <aitos/types.h>
#include <aitos/console.h>

int __init fs_init_stub(void)
{
	console_puts("fs: stub layer ready\n");
	return 0;
}

void fs_set_ops(const struct fs_ops *ops)
{
	(void)ops;
}
