/* SPDX-License-Identifier: Apache-2.0 */
#include <aitos/printk.h>
#include <aitos/console.h>

#include <stdarg.h>

static void emit_char(char c)
{
	console_write_char(c);
}

static void emit_string(const char *s)
{
	while (*s)
		emit_char(*s++);
}

static void emit_decimal(unsigned long value)
{
	char buf[21];
	int i = 20;

	buf[i] = '\0';
	if (!value) {
		emit_char('0');
		return;
	}
	while (value) {
		buf[--i] = '0' + (value % 10);
		value /= 10;
	}
	emit_string(&buf[i]);
}

int printk(const char *fmt, ...)
{
	va_list ap;
	const char *p;

	va_start(ap, fmt);
	for (p = fmt; *p; p++) {
		if (*p != '%') {
			emit_char(*p);
			continue;
		}
		p++;
		switch (*p) {
		case 's':
			emit_string(va_arg(ap, const char *));
			break;
		case 'd':
			emit_decimal((unsigned long)va_arg(ap, int));
			break;
		case 'l':
			if (p[1] == 'l' && p[2] == 'u') {
				emit_decimal((unsigned long)va_arg(ap, unsigned long long));
				p += 2;
			} else if (p[1] == 'u') {
				emit_decimal((unsigned long)va_arg(ap, unsigned long));
				p += 1;
			} else {
				emit_char(*p);
			}
			break;
		case 'c':
			emit_char((char)va_arg(ap, int));
			break;
		case '%':
			emit_char('%');
			break;
		default:
			emit_char(*p);
			break;
		}
	}
	va_end(ap);
	return 0;
}
