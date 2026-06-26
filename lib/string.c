/* SPDX-License-Identifier: Apache-2.0 */
#include <aitos/types.h>

void *memset(void *dst, int value, size_t size)
{
	u8 *p = dst;

	while (size--)
		*p++ = (u8)value;
	return dst;
}

void *memcpy(void *dst, const void *src, size_t size)
{
	u8 *d = dst;
	const u8 *s = src;

	while (size--)
		*d++ = *s++;
	return dst;
}

int memcmp(const void *a, const void *b, size_t size)
{
	const u8 *x = a;
	const u8 *y = b;

	while (size--) {
		if (*x != *y)
			return *x - *y;
		x++;
		y++;
	}
	return 0;
}

char *strcpy(char *dst, const char *src)
{
	char *ret = dst;

	while ((*dst++ = *src++))
		;
	return ret;
}

size_t strlen(const char *str)
{
	const char *p = str;

	while (*p)
		p++;
	return p - str;
}

int strcmp(const char *a, const char *b)
{
	while (*a && *a == *b) {
		a++;
		b++;
	}
	return *a - *b;
}

char *strchr(const char *s, int c)
{
	while (*s) {
		if (*s == (char)c)
			return (char *)s;
		s++;
	}
	return NULL;
}

int strncmp(const char *a, const char *b, size_t n)
{
	while (n && *a && (*a == *b)) {
		a++;
		b++;
		n--;
	}
	if (!n)
		return 0;
	return *a - *b;
}
