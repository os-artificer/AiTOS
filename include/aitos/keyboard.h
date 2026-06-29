/* SPDX-License-Identifier: Apache-2.0 */
#ifndef _AITOS_KEYBOARD_H
#define _AITOS_KEYBOARD_H

#include <aitos/types.h>
#include <aitos/compiler.h>

int __init keyboard_init(void);
void keyboard_reinit_com1(void);
void keyboard_drain_com1(void);
int keyboard_read_char(void);

#endif /* _AITOS_KEYBOARD_H */
