/* SPDX-License-Identifier: Apache-2.0 */
#ifndef _AITOS_BOOT_INFO_H
#define _AITOS_BOOT_INFO_H

#include <aitos/types.h>

#define AITOS_BOOT_INFO_MAGIC	0x4149544f535f494eULL /* "AITOS_IN" */
#define AITOS_BOOT_INFO_PHYS	0x800

struct boot_info {
	u64	magic;
	u64	mem_bytes;
	u64	kernel_phys;
	u64	reserved;
};

static inline struct boot_info *boot_info_ptr(void)
{
	return (struct boot_info *)(unsigned long)AITOS_BOOT_INFO_PHYS;
}

#endif /* _AITOS_BOOT_INFO_H */
