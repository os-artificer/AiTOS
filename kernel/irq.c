/* SPDX-License-Identifier: Apache-2.0 */
#include <aitos/irq.h>
#include <aitos/io.h>
#include <aitos/boot.h>

#include <aitos/printk.h>
#include <aitos/console.h>
#include <aitos/types.h>
#include <aitos/boot.h>

#define IDT_DESC_INTERRUPT	0x0e

#define PIC_M_CTRL	0x20
#define PIC_M_DATA	0x21
#define PIC_S_CTRL	0xa0
#define PIC_S_DATA	0xa1

extern int printk_use_serial;

static u16 idt_code_selector(void)
{
	u16 cs;

	asm volatile("movw %%cs, %0" : "=r"(cs));
	return cs;
}

#define IDT_DESC_P	0x80
#define IDT_DESC_DPL0	0x00
#define IDT_DESC_TRAP		0x0f
#define PIC_EOI		0x20

static void pic_send_eoi(u8 vector)
{
	/* EOI must be written to the COMMAND port, not the data/mask port. */
	if (vector >= 0x28)
		outb(PIC_EOI, PIC_S_CTRL);
	outb(PIC_EOI, PIC_M_CTRL);
}

struct gate_desc64 {
	u16	offset_low;
	u16	selector;
	u8	ist;
	u8	type_attr;
	u16	offset_mid;
	u32	offset_high;
	u32	reserved;
} __attribute__((packed));

struct idt_ptr {
	u16	limit;
	u64	base;
} __attribute__((packed));

static struct gate_desc64 idt[IDT_VECTORS];
static irq_handler_t irq_table[IDT_VECTORS];
static struct idt_ptr idtr;

extern u64 intr_entry_table[];

static void pic_init(void)
{
	outb(0xff, PIC_M_DATA);
	outb(0xff, PIC_S_DATA);

	outb(0x11, PIC_M_CTRL);
	outb(0x20, PIC_M_DATA);
	outb(0x04, PIC_M_DATA);
	outb(0x01, PIC_M_DATA);

	outb(0x11, PIC_S_CTRL);
	outb(0x28, PIC_S_DATA);
	outb(0x02, PIC_S_DATA);
	outb(0x01, PIC_S_DATA);

	/* Keep all IRQs masked until intr_enable() in kmain. */
	outb(0xff, PIC_M_DATA);
	outb(0xff, PIC_S_DATA);
}

static void __attribute__((noinline)) set_gate(int vec, void *handler)
{
	u64 addr = (u64)(uintptr_t)handler;
	u8 type = (vec < 0x20) ? IDT_DESC_TRAP : IDT_DESC_INTERRUPT;

	idt[vec].offset_low = addr & 0xffff;
	idt[vec].selector = idt_code_selector();
	idt[vec].ist = 0;
	idt[vec].type_attr = IDT_DESC_P | IDT_DESC_DPL0 | type;
	idt[vec].offset_mid = (addr >> 16) & 0xffff;
	idt[vec].offset_high = (addr >> 32) & 0xffffffff;
	idt[vec].reserved = 0;
}

void irq_dispatch(u64 vector)
{
	if (vector < IDT_VECTORS && irq_table[vector])
		irq_table[vector]();
	if (vector >= 0x20 && vector < 0x40)
		pic_send_eoi((u8)vector);
}

void register_irq_handler(u8 vector, irq_handler_t handler)
{
	if (vector < IDT_VECTORS)
		irq_table[vector] = handler;
}

void irq_init(void)
{
	int i;
	u16 cs;

	asm volatile("movw %%cs, %0" : "=r"(cs));
	for (i = 0; i < 0x30; i++) {
		u64 addr = (u64)(uintptr_t)intr_entry_table[i];
		u8 type = (i < 0x20) ? IDT_DESC_TRAP : IDT_DESC_INTERRUPT;

		idt[i].offset_low = addr & 0xffff;
		idt[i].selector = cs;
		idt[i].ist = 0;
		idt[i].type_attr = IDT_DESC_P | IDT_DESC_DPL0 | type;
		idt[i].offset_mid = (addr >> 16) & 0xffff;
		idt[i].offset_high = (addr >> 32) & 0xffffffff;
		idt[i].reserved = 0;
	}

	idtr.limit = sizeof(idt) - 1;
	idtr.base = (u64)(uintptr_t)idt;
	asm volatile("lidt %0" : : "m"(idtr));
	pic_init();
	console_puts("irq: IDT and PIC initialized\n");
}

enum intr_status intr_enable(void)
{
	/* GRUB headless: COM1 only; keep PS/2 masked to avoid spurious IRQ1 flood. */
	if (boot_is_multiboot2())
		outb(0xff, PIC_M_DATA);
	else
		outb(0xf9, PIC_M_DATA);
	outb(0xff, PIC_S_DATA);
	asm volatile("sti");
	return INTR_ON;
}

enum intr_status intr_disable(void)
{
	asm volatile("cli" ::: "memory");
	return INTR_OFF;
}

enum intr_status intr_get_status(void)
{
	u64 flags;

	asm volatile("pushfq; pop %0" : "=r"(flags));
	return (flags & (1 << 9)) ? INTR_ON : INTR_OFF;
}
