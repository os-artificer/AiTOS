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

void boot_info_load(struct boot_info *dst);

#endif /* _AITOS_BOOT_INFO_H */
