/*
 * guzhoudiaoke@126.com
 * 2017-10-22
 */

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include "types.h"
#include "screen.h"
#include "spinlock.h"

typedef char* va_list;
#define va_start(ap,p)      (ap = (char *) (&(p)+1))
#define va_arg(ap, type)    ((type *) (ap += sizeof(type)))[-1]
#define va_end(ap)

#define CHARACTER(ch)       (ch & 0xff)
#define MAX_ROW             128
#define MAX_COL             48
#define BACKGROUND_COLOR    RGB(0x40, 0, 0x30)
#define CURSOR_COLOR        RGB(0xff, 0xff, 0x00)

class BabyOS;
class console_t {
public:
	console_t();
	~console_t();

	void init();
	void kprintf(color_ref_t color, const char *fmt, ...);
    void update();

private:
	void draw_background();
    void draw_cursor();
    void draw_time();

	void put_char(char c, color_ref_t color);
    void unput_char();
    void backspace();
	void putc(int c, color_ref_t color);
	void print_int(int32 n, int32 base, int32 sign, color_ref_t color);

	uint32 m_row_num;
	uint32 m_col_num;
	uint32 m_row;
	uint32 m_col;
	char m_text[MAX_ROW][MAX_COL];

    uint32 m_tick_to_update;
    spinlock_t m_lock;
};

#endif
