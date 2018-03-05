/*
 * guzhoudiaoke@126.com
 * 2017-10-22
 */

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include "types.h"
#include "screen.h"
#include "spinlock.h"
#include "waitqueue.h"

#define BUFFER_SIZE     1024

#define CHARACTER(ch)       (ch & 0xff)
#define MAX_ROW             48
#define MAX_COL             128
#define BACKGROUND_COLOR    RGB(0x40, 0, 0x30)
#define CURSOR_COLOR        RGB(0xff, 0xff, 0x00)

typedef struct color_text_s {
    char ch;
    color_ref_t color;
} color_text_t;

typedef struct input_buffer_s {
    void init();
    void input(char ch);

    unsigned    m_read_index;
    unsigned    m_write_index;
    unsigned    m_edit_index;
    char        m_buffer[BUFFER_SIZE];
} input_buffer_t;

class console_t {
public:
	console_t();
	~console_t();

	void init();
	void kprintf(color_ref_t color, const char *fmt, ...);
    void update();
    void do_input(char ch);
    int  read(void* buf, int size);
    int  write(void* buf, int size);
	void putc(int c, color_ref_t color);
    void wakeup_reader();

private:
	void draw_background();
    void draw_cursor();
    void draw_time();

	void put_char(char c, color_ref_t color);
    void unput_char();
    void backspace();
	void print_int(int32 n, int32 base, int32 sign, color_ref_t color);
    void scroll();

	uint32          m_row_num;
	uint32          m_col_num;
	uint32          m_row;
	uint32          m_col;
    color_text_t    m_text[MAX_ROW][MAX_COL];
    uint32          m_tick_to_update;
    bool            m_show_cursor;
    spinlock_t      m_lock;
    input_buffer_t  m_input_buffer;
    wait_queue_t    m_wait_queue;
};

#endif
