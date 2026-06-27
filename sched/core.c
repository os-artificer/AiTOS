/* SPDX-License-Identifier: Apache-2.0 */
#include <aitos/sched.h>

#include <aitos/printk.h>

static void stub_sched_init(void)
{
}

static void stub_schedule(void)
{
}

static void stub_yield(void)
{
}

static const struct sched_class stub_sched_class = {
	.sched_init = stub_sched_init,
	.schedule = stub_schedule,
	.yield = stub_yield,
};

static const struct sched_class *sched_class;

void sched_set_class(const struct sched_class *class)
{
	sched_class = class;
}

int __init sched_init(void)
{
	sched_set_class(&stub_sched_class);
	if (sched_class && sched_class->sched_init)
		sched_class->sched_init();
	pr_info("sched: stub layer ready\n");
	return 0;
}
