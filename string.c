/*  string.c -- standard C string-manipulation functions.

Copyright (C) 2008		Segher Boessenkool <segher@kernel.crashing.org>
Copyright (C) 2009		Haxx Enterprises <bushing@gmail.com>

Portions taken from the Public Domain C Library (PDCLib).
https://negix.net/trac/pdclib

# This code is licensed to you under the terms of the GNU GPL, version 2;
# see file COPYING or http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
*/

#include "string.h"
#include "malloc.h"

size_t strlen(const char *s)
{
	size_t len;

	for (len = 0; s[len]; len++)
		;

	return len;
}

size_t strnlen(const char *s, size_t count)
{
	size_t len;

	for (len = 0; s[len] && len < count; len++)
		;

	return len;
}

char *strdup(char *s) {
	size_t l = strlen(s)+1;
	char *r = malloc(l);
	memcpy(r, s, l);
	return r;
}

void *memset(void *b, int c, size_t len)
{
	size_t i;

	for (i = 0; i < len; i++)
		((unsigned char *)b)[i] = c;

	return b;
}

void *memcpy(void *dst, const void *src, size_t len)
{
	size_t i;

	for (i = 0; i < len; i++)
		((unsigned char *)dst)[i] = ((unsigned char *)src)[i];

	return dst;
}

int memcmp(const void *s1, const void *s2, size_t len)
{
	size_t i;
	const unsigned char * p1 = (const unsigned char *) s1;
	const unsigned char * p2 = (const unsigned char *) s2;

	for (i = 0; i < len; i++)
		if (p1[i] != p2[i]) return p1[i] - p2[i];
	
	return 0;
}

int strcmp(const char *s1, const char *s2)
{
	size_t i;

	for (i = 0; s1[i] && s1[i] == s2[i]; i++)
		;
	
	return s1[i] - s2[i];
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	size_t i;

	for (i = 0; i < n && s1[i] && s1[i] == s2[i]; i++)
		;
	if (i == n) return 0;
	return s1[i] - s2[i];
}

size_t strlcpy(char *dest, const char *src, size_t maxlen)
{
	size_t len,needed;

	len = needed = strnlen(src, maxlen-1) + 1;
	if (len >= maxlen)
		len = maxlen-1;

	memcpy(dest, src, len);
	dest[len]='\0';

	return needed-1;
}

size_t strlcat(char *dest, const char *src, size_t maxlen)
{
	size_t used;

    used = strnlen(dest, maxlen-1);
	return used + strlcpy(dest + used, src, maxlen - used);
}

char * strchr(const char *s, char c)
{
	size_t i;
	
	for (i = 0; s[i]; i++)
		if (s[i] == c) return (char *)s + i;

	return NULL;
}

char * strchr2(const char *s, char c1, char c2)
{
	size_t i;
	
	for (i = 0; s[i]; i++) {
		if (s[i] == c1) {
			if (!s[i+1])
				break;
			if (s[i+1] == c2)
				return (char *)s + i;
		}
	}

	return NULL;
}

size_t strspn(const char *s1, const char *s2)
{
	size_t len = 0;
	const char *p;

	while (s1[len]) {
		p = s2;
		while (*p) {
			if (s1[len] == *p)
				break;

			++p;
		}
		if (!*p)
			return len;

		++len;
	}

	return len;
}

size_t strcspn(const char *s1, const char *s2)
{
	size_t len = 0;
	const char *p;

	while (s1[len]) {
		p = s2;
		while (*p)
			if (s1[len] == *p++)
				return len;

		++len;
	}

	return len;
}

