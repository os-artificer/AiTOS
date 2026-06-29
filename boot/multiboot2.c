/* SPDX-License-Identifier: Apache-2.0 */
#include <aitos/boot.h>
#include <aitos/boot_info.h>

#include <aitos/errno.h>

#define MB2_TAG_MMAP	6
#define MB2_TAG_MAX_WALK 64

static void boot_info_fill(struct boot_info *dst, u64 mem_bytes)
{
	volatile struct boot_info *out = (volatile struct boot_info *)dst;

	out->magic = AITOS_BOOT_INFO_MAGIC;
	out->mem_bytes = mem_bytes ? mem_bytes : (128ULL << 20);
	out->kernel_phys = 0x100000;
	out->reserved = 0;
}

int __attribute__((noinline, optimize("O0"))) boot_multiboot2_parse(u64 mbi_phys,
								   struct boot_info *dst)
{
	const volatile u8 *base = (const volatile u8 *)(uintptr_t)mbi_phys;
	const volatile u32 *words = (const volatile u32 *)(uintptr_t)mbi_phys;
	u32 total_size;
	u64 mem_bytes = 0;
	u32 off;
	unsigned steps = 0;

	if (!mbi_phys || !dst)
		return -EINVAL;

	total_size = words[0];
	if (total_size < 16 || (total_size & 7)) {
		boot_info_fill(dst, 0);
		return 0;
	}

	for (off = 8; off + 8 <= total_size && steps++ < MB2_TAG_MAX_WALK; ) {
		u32 type = words[off / 4];
		u32 size = words[off / 4 + 1];
		u32 next;

		if (size < 8 || off + size > total_size)
			break;

		if (type == MB2_TAG_MMAP && size >= 16) {
			u32 esize = words[off / 4 + 2];
			u32 count;
			u32 i;

			if (esize >= 24 && esize <= size) {
				count = (size - 16) / esize;
				if (count > 32)
					count = 32;

				for (i = 0; i < count; i++) {
					u32 eoff = off + 16 + i * esize;

					if (eoff + 20 <= off + size &&
					    words[eoff / 4 + 4] == 1)
						mem_bytes += *(const volatile u64 *)(base + eoff + 8);
				}
			}
			break;
		}

		next = off + ((size + 7) & ~7U);
		if (next <= off)
			break;
		off = next;
	}

	boot_info_fill(dst, mem_bytes);
	return 0;
}
