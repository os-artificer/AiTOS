/* SPDX-License-Identifier: Apache-2.0 */
#include <aitos/printk.h>
#include <aitos/console.h>
#include <aitos/io.h>

#include <stdarg.h>

extern int printk_use_serial;

static int printk_serial_only(void)
{
	u16 cs;

	asm volatile("movw %%cs, %0" : "=r"(cs));
	return cs == 0x10 || printk_use_serial;
}

static void emit_serial_char(char c)
{
	u8 ch = (u8)c;

	asm volatile("outb %0, $0xe9" : : "a"(ch));
	if (ch == '\n') {
		u8 cr = '\r';

		asm volatile("outb %0, $0xe9" : : "a"(cr));
	}
}

static void emit_char(char c, int serial_only)
{
	if (serial_only)
		emit_serial_char(c);
	else
		console_write_char(c);
}

static void emit_string(const char *s, int serial_only)
{
	while (*s)
		emit_char(*s++, serial_only);
}

static void emit_decimal(unsigned long value, int serial_only)
{
	char buf[21];
	int i = 20;

	buf[i] = '\0';
	if (!value) {
		emit_char('0', serial_only);
		return;
	}
	while (value) {
		buf[--i] = '0' + (value % 10);
		value /= 10;
	}
	emit_string(&buf[i], serial_only);
}

int __attribute__((optimize("O0"))) printk(const char *fmt, ...)
{
	va_list ap;
	const char *p;
	int serial_only = printk_serial_only();

	{
		register unsigned char _al __asm__("al") = 0;

		(void)_al;
		va_start(ap, fmt);
	}
	for (p = fmt; *p; p++) {
		if (*p != '%') {
			emit_char(*p, serial_only);
			continue;
		}
		p++;
		switch (*p) {
		case 's':
			emit_string(va_arg(ap, const char *), serial_only);
			break;
		case 'd':
			emit_decimal((unsigned long)va_arg(ap, int), serial_only);
			break;
		case 'l':
			if (p[1] == 'l' && p[2] == 'u') {
				emit_decimal((unsigned long)va_arg(ap, unsigned long long),
					     serial_only);
				p += 2;
			} else if (p[1] == 'u') {
				emit_decimal((unsigned long)va_arg(ap, unsigned long),
					     serial_only);
				p += 1;
			} else {
				emit_char(*p, serial_only);
			}
			break;
		case 'c':
			emit_char((char)va_arg(ap, int), serial_only);
			break;
		case '%':
			emit_char('%', serial_only);
			break;
		default:
			emit_char(*p, serial_only);
			break;
		}
	}
	va_end(ap);
	return 0;
}
