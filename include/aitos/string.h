/* SPDX-License-Identifier: Apache-2.0 */
#ifndef _AITOS_STRING_H
#define _AITOS_STRING_H

#include <aitos/types.h>

void *memset(void *dst, int value, size_t size);
void *memcpy(void *dst, const void *src, size_t size);
int memcmp(const void *a, const void *b, size_t size);
char *strcpy(char *dst, const char *src);
size_t strlen(const char *s);
int strcmp(const char *a, const char *b);
char *strchr(const char *s, int c);
int strncmp(const char *a, const char *b, size_t n);

#endif /* _AITOS_STRING_H */
