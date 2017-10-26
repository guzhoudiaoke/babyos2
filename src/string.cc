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
    char *d = (char *) dst;
    for (uint32 i = 0; i < n; i++) {
        *d++ = (c & 0xff);
    }

    return dst;
}

