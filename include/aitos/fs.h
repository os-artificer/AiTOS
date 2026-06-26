/* SPDX-License-Identifier: Apache-2.0 */
#ifndef _AITOS_FS_H
#define _AITOS_FS_H

#include <aitos/compiler.h>

struct fs_ops {
	int (*init)(void);
};

void fs_set_ops(const struct fs_ops *ops);
int __init fs_init_stub(void);

#endif /* _AITOS_FS_H */
