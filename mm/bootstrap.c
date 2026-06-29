/* SPDX-License-Identifier: Apache-2.0 */
#include <aitos/mm.h>
#include <aitos/boot.h>
#include <aitos/boot_info.h>

#include <aitos/console.h>
#include <aitos/errno.h>

static const struct mm_ops *mm_ops;

static u64 boot_phys_read(uintptr_t phys)
{
	u64 val;

	__asm__ __volatile__(
		"movq (%[addr]), %[val]"
		: [val] "=r" (val)
		: [addr] "r" (phys)
		: "memory");
	return val;
}

void boot_info_load(struct boot_info *dst)
{
	uintptr_t base = AITOS_BOOT_INFO_PHYS;

	dst->magic = boot_phys_read(base + __builtin_offsetof(struct boot_info, magic));
	dst->mem_bytes = boot_phys_read(base + __builtin_offsetof(struct boot_info, mem_bytes));
	dst->kernel_phys = boot_phys_read(base + __builtin_offsetof(struct boot_info, kernel_phys));
	dst->reserved = boot_phys_read(base + __builtin_offsetof(struct boot_info, reserved));
}

static int bootstrap_init(const struct boot_info *bi)
{
	u16 cs;

	asm volatile("movw %%cs, %0" : "=r"(cs));
	if (cs == 0x10) {
		console_puts("mm: bootstrap 128 MB RAM (stub)\n");
		return 0;
	}
	if (!bi || bi->magic != AITOS_BOOT_INFO_MAGIC) {
		console_puts("ERROR: mm: invalid boot_info\n");
		return -EINVAL;
	}
	console_puts("mm: bootstrap 128 MB RAM (stub)\n");
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
	return bootstrap_init(boot_info_get());
}
