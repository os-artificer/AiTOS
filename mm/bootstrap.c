/* SPDX-License-Identifier: Apache-2.0 */
#include <aitos/mm.h>
#include <aitos/boot_info.h>

#include <aitos/printk.h>
#include <aitos/errno.h>

static const struct mm_ops *mm_ops;

static int bootstrap_init(const struct boot_info *bi)
{
	if (!bi || bi->magic != AITOS_BOOT_INFO_MAGIC) {
		pr_err("mm: invalid boot_info\n");
		return -EINVAL;
	}
	pr_info("mm: bootstrap %llu MB RAM (stub)\n", bi->mem_bytes >> 20);
	return 0;
}

static const struct mm_ops bootstrap_mm_ops = {
	.init = bootstrap_init,
	.alloc_pages = NULL,
	.free_pages = NULL,
};

void mm_set_ops(const struct mm_ops *ops)
{
	mm_ops = ops;
}

int __init mm_init_bootstrap(void)
{
	mm_set_ops(&bootstrap_mm_ops);
	if (mm_ops && mm_ops->init)
		return mm_ops->init(boot_info_ptr());
	return 0;
}
