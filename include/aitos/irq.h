/* SPDX-License-Identifier: Apache-2.0 */
#ifndef _AITOS_IRQ_H
#define _AITOS_IRQ_H

#include <aitos/types.h>

typedef void (*irq_handler_t)(void);

enum intr_status {
	INTR_OFF,
	INTR_ON,
};

#define IDT_VECTORS	48

void irq_init(void);
void register_irq_handler(u8 vector, irq_handler_t handler);
enum intr_status intr_enable(void);
enum intr_status intr_disable(void);
enum intr_status intr_get_status(void);

#endif /* _AITOS_IRQ_H */
