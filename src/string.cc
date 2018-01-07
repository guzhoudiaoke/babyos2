/*
 * guzhoudiaoke@126.com
 * 2017-10-22
 */

#include "types.h"
#include "string.h"

void* memmov(void *dst, const void *src, uint32 n)
{
    const char *s = (const char *) src;
    char *d = (char *) dst;

    if (s < d && s + n > d) {
        s += n, d += n;
        while (n--) {
            *--d = *--s;
        }
    }
    else {
        while (n--) {
            *d++ = *s++;
        }
    }

    return dst;
}

void* memcpy(void *dst, const void *src, uint32 n)
{
    return memmov(dst, src, n);
}

void* memset(void *dst, uint32 c, uint32 n)
{
    char* d = (char *) dst;
    for (uint32 i = 0; i < n; i++) {
        *d++ = (c & 0xff);
    }

    return dst;
}

char* strcpy(char* dst, const char* src)
{
	char* d = dst;
	while (*src) {
		*d++ = *src++;
	}
    *d++ = '\0';

	return dst;
}

char* strncpy(char* dst, const char* src, int n)
{
	char* d = dst;
	while (--n >= 0 && *src) {
		*d++ = *src++;
	}
    *d++ = '\0';

	return dst;
}

int strcmp(const char* s1, const char *s2)
{
    while (*s1 && *s2 && *s1 == *s2) {
        s1++;
        s2++;
    }

    return *s1 - *s2;
}

int strlen(const char* s)
{
    int len = 0;
    while (*s++) {
        len++;
    }
    return len;
}

