#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>

#include "util.h"

/*
 * This is sqrt(SIZE_MAX+1), as s1*s2 <= SIZE_MAX
 * if both s1 < MUL_NO_OVERFLOW and s2 < MUL_NO_OVERFLOW
 */
#define MUL_NO_OVERFLOW	((size_t)1 << (sizeof(size_t) * 4))

void *
reallocarray(void *optr, size_t nmemb, size_t size)
{
	if ((nmemb >= MUL_NO_OVERFLOW || size >= MUL_NO_OVERFLOW) &&
	    nmemb > 0 && SIZE_MAX / nmemb < size) {
		errno = ENOMEM;
		return NULL;
	}
	return realloc(optr, size * nmemb);
}

char *
strchrnul(const char *str, int chr)
{
	while (*str && *str != chr) ++str;
	return (char *) str;
}

char *
strfmt(const char *fmt, ...)
{
	char buf[1000];
	int len;
	va_list ap;
	
	va_start(ap, fmt);
	len = vsnprintf(buf, 1000, fmt, ap);
	assert(len < 1000);
	va_end(ap);
	return strdup(buf);
}

static uint32_t
fnv1a(const unsigned char *bytes, size_t len)
{
	uint32_t hash = 2166136261;
	size_t i;

	for (i = 0; i < len; ++i) {
		hash ^= bytes[i];
		hash *= 16777619;
	}
	
	return hash;
}

uint32_t
memhash(const void *mem, size_t len)
{
	return fnv1a(mem, len);
}

uint32_t
strhash(const char *str)
{
	return fnv1a((const void *) str, strlen(str));
}

uint32_t
xorfold(uint32_t orig, int bits)
{
	uint32_t value = 0;

	do {
		value ^= orig;
		orig >>= bits;
	} while (orig);
	
	value &= (1u << bits) - 1u;
	
	return value;
}

