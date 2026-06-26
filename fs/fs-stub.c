/* SPDX-License-Identifier: Apache-2.0 */
#include <aitos/fs.h>

#include <aitos/types.h>
#include <aitos/printk.h>

static const struct fs_ops noop_fs = {
	.init = NULL,
};

static const struct fs_ops *fs_ops = &noop_fs;

void fs_set_ops(const struct fs_ops *ops)
{
	if (ops)
		fs_ops = ops;
}

int __init fs_init_stub(void)
{
	if (fs_ops && fs_ops->init)
		fs_ops->init();
	pr_info("fs: stub layer ready\n");
	return 0;
}
