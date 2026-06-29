/* SPDX-License-Identifier: Apache-2.0 */
#include <aitos/boot.h>
#include <aitos/boot_info.h>

#include <aitos/errno.h>

extern void boot_info_load(struct boot_info *dst);
extern int boot_multiboot2_parse(u64 mbi_phys, struct boot_info *dst);

extern volatile u64 boot_handoff_source;
extern volatile u64 boot_handoff_mbi_phys;

static struct boot_info boot_info_stash;
static int boot_info_valid;

static int mbi_phys_plausible(u64 phys)
{
	return phys >= 0x10000 && phys < (128ULL << 20);
}

u64 boot_handoff_source_get(void)
{
	return boot_handoff_source;
}

static void __attribute__((noinline)) boot_info_default(struct boot_info *dst)
{
	volatile struct boot_info *out = (volatile struct boot_info *)dst;

	out->magic = AITOS_BOOT_INFO_MAGIC;
	out->mem_bytes = 128ULL << 20;
	out->kernel_phys = 0x100000;
	out->reserved = 0;
}

static int grub_cs_selector(u16 cs)
{
	return cs == 0x10 || cs == 0x8;
}

void boot_early_init(u64 source, u64 mbi_phys)
{
	int err;
	u16 cs;

	asm volatile("movw %%cs, %0" : "=r"(cs));

	boot_info_valid = 0;

	if (grub_cs_selector(cs)) {
		boot_info_default(&boot_info_stash);
		boot_handoff_source = BOOT_SOURCE_MULTIBOOT2;
		boot_info_valid = 1;
		return;
	}

	if (source == BOOT_SOURCE_MULTIBOOT2 && mbi_phys_plausible(mbi_phys)) {
		err = boot_multiboot2_parse(mbi_phys, &boot_info_stash);
		if (!err) {
			boot_info_valid = 1;
			return;
		}
		boot_info_default(&boot_info_stash);
		boot_info_valid = 1;
		return;
	}

	if (source == BOOT_SOURCE_LEGACY) {
		boot_info_load(&boot_info_stash);
		if (boot_info_stash.magic == AITOS_BOOT_INFO_MAGIC) {
			boot_info_valid = 1;
			return;
		}
	}

	if (mbi_phys_plausible(mbi_phys)) {
		err = boot_multiboot2_parse(mbi_phys, &boot_info_stash);
		if (!err) {
			boot_handoff_source = BOOT_SOURCE_MULTIBOOT2;
			boot_info_valid = 1;
			return;
		}
	}

	if (boot_handoff_source != BOOT_SOURCE_MULTIBOOT2 && mbi_phys_plausible(mbi_phys)) {
		boot_handoff_source = BOOT_SOURCE_MULTIBOOT2;
		boot_info_default(&boot_info_stash);
		boot_info_valid = 1;
		return;
	}

	boot_info_default(&boot_info_stash);
	boot_info_valid = 1;
}

const struct boot_info *boot_info_get(void)
{
	return boot_info_valid ? &boot_info_stash : NULL;
}

int boot_is_multiboot2(void)
{
	if (boot_handoff_source == BOOT_SOURCE_MULTIBOOT2)
		return 1;
	u16 cs;

	asm volatile("movw %%cs, %0" : "=r"(cs));
	return grub_cs_selector(cs);
}
