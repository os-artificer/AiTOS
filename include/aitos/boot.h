/* SPDX-License-Identifier: Apache-2.0 */
#ifndef _AITOS_BOOT_H
#define _AITOS_BOOT_H

#include <aitos/types.h>
#include <aitos/boot_info.h>

#define BOOT_SOURCE_LEGACY	0
#define BOOT_SOURCE_MULTIBOOT2	1

#define MULTIBOOT2_BOOTLOADER_MAGIC	0x36d76289ULL

extern volatile u64 boot_handoff_source;
extern volatile u64 boot_handoff_mbi_phys;
extern int printk_use_serial;

void boot_early_init(u64 source, u64 mbi_phys);
const struct boot_info *boot_info_get(void);
int boot_is_multiboot2(void);
u64 boot_handoff_source_get(void);

#endif /* _AITOS_BOOT_H */
