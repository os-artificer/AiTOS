/* SPDX-License-Identifier: Apache-2.0 */
#ifndef _AITOS_COMPILER_H
#define _AITOS_COMPILER_H

#define __used		__attribute__((__used__))
#define __unused	__attribute__((__unused__))
#define __init		__attribute__((__section__(".init.text")))
#define __initdata	__attribute__((__section__(".init.data")))

#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)

#define barrier()	asm volatile("" ::: "memory")

#endif /* _AITOS_COMPILER_H */
