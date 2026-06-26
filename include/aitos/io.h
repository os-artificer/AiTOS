/* SPDX-License-Identifier: Apache-2.0 */
#ifndef _AITOS_IO_H
#define _AITOS_IO_H

#include <aitos/types.h>

static inline void outb(u8 value, u16 port)
{
	asm volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline u8 inb(u16 port)
{
	u8 value;

	asm volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
	return value;
}

#endif /* _AITOS_IO_H */
