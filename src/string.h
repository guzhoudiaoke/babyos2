/*
 * guzhoudiaoke@126.com
 * 2017-10-22
 */

#ifndef _STRING_H_
#define _STRING_H_

void* memmov(void* dst, const void* src, uint32 n);
void* memcpy(void* dst, const void* src, uint32 n);
void* memset(void* dst, uint32 c, uint32 n);

char* strcpy(char* dst, const char* src);

#endif
