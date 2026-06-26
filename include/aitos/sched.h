/* SPDX-License-Identifier: Apache-2.0 */
#ifndef _AITOS_SCHED_H
#define _AITOS_SCHED_H

#include <aitos/compiler.h>

struct sched_class {
	void (*sched_init)(void);
	void (*schedule)(void);
	void (*yield)(void);
};

void sched_set_class(const struct sched_class *class);
int __init sched_init(void);

#endif /* _AITOS_SCHED_H */
