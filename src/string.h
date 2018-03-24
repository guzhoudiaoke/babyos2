/*
 * guzhoudiaoke@126.com
 * 2017-10-22
 */

#ifndef _STRING_H_
#define _STRING_H_

#include "arg.h"

void* memmov(void* dst, const void* src, uint32 n);
void* memcpy(void* dst, const void* src, uint32 n);
void* memset(void* dst, uint32 c, uint32 n);
int   memcmp(const void* b1, const void* b2, uint32 n);

char* strcpy(char* dst, const char* src);
char* strncpy(char* dst, const char* src, int n);
int   strcmp(const char* s1, const char *s2);
int   strlen(const char* s);
int   strncmp(const char* s1, const char *s2, int n);

bool  is_digit(char c);
int   sprintf(char* buffer, const char *fmt, ...);
int   vsprintf(char *buffer, const char *fmt, va_list ap);

#endif
