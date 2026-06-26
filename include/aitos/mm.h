/* SPDX-License-Identifier: Apache-2.0 */
#ifndef _AITOS_MM_H
#define _AITOS_MM_H

#include <aitos/types.h>
#include <aitos/compiler.h>
#include <aitos/boot_info.h>

struct mm_ops {
	int (*init)(const struct boot_info *bi);
	void *(*alloc_pages)(unsigned int order);
	void (*free_pages)(void *addr, unsigned int order);
};

void mm_set_ops(const struct mm_ops *ops);
int __init mm_init_bootstrap(void);

#endif /* _AITOS_MM_H */
