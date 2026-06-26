/* SPDX-License-Identifier: Apache-2.0 */
#ifndef _AITOS_PRINTK_H
#define _AITOS_PRINTK_H

#include <aitos/compiler.h>

int printk(const char *fmt, ...);

#define pr_info(fmt, ...)	printk(KERN_INFO fmt, ##__VA_ARGS__)
#define pr_err(fmt, ...)	printk(KERN_ERR fmt, ##__VA_ARGS__)

#define KERN_INFO	""
#define KERN_ERR	"ERROR: "

#endif /* _AITOS_PRINTK_H */
