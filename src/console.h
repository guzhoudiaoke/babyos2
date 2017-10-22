/*
 * guzhoudiaoke@126.com
 * 2017-10-22
 */

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include "types.h"
#include "screen.h"

typedef char * va_list;
#define va_start(ap,p)      (ap = (char *) (&(p)+1))
#define va_arg(ap, type)    ((type *) (ap += sizeof(type)))[-1]
#define va_end(ap)

#define CHARACTER(ch)       (ch & 0xff)

#define MAX_ROW             128
#define MAX_COL             48

#define BACKGROUND_COLOR    RGB(0x40, 0, 0x30)

typedef struct console_s {
    uint32 row_num;
    uint32 col_num;
    uint32 row;
    uint32 col;
    char text[MAX_ROW][MAX_COL];
} console_t;

void init_console();
void kprintf(color_ref_t color, char *fmt, ...);

#endif
