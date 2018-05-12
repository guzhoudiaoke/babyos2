/*
 * guzhoudiaoke@126.com
 * 2017-10-22
 */

#include "babyos.h"
#include "console.h"
#include "screen.h"
#include "string.h"
#include "timer.h"
#include "x86.h"

static int console_read(inode_t* inode, void* buf, int size)
{
    return console()->read(buf, size);
}

static int console_write(inode_t* inode, void* buf, int size)
{
    return console()->write(buf, size);
}

/****************************************************************/

void input_buffer_t::init()
{
    m_read_index = 0;
    m_write_index = 0;
    m_edit_index = 0;
    memset(m_buffer, 0, BUFFER_SIZE);
}

void input_buffer_t::input(char ch)
{
    if (ch == '\b') {
        if (m_edit_index == m_write_index) {
            return;
        }
        m_edit_index--;
    }
    else if (ch == '\n') {
        m_buffer[m_edit_index++ % BUFFER_SIZE] = ch;
        m_write_index = m_edit_index;
        console()->wakeup_reader();
    }
    else {
        m_buffer[m_edit_index++ % BUFFER_SIZE] = ch;
    }
    console()->write(&ch, 1);
}

/****************************************************************/

console_t::console_t()
{
}
console_t::~console_t()
{
}

void console_t::draw_background()
{
    rect_t rc = { 0, 0, os()->get_screen()->width(), os()->get_screen()->height() };
    os()->get_screen()->fill_rectangle(rc, BACKGROUND_COLOR);
}

void console_t::draw_cursor()
{
    rect_t rc = { m_col*ASC16_WIDTH, m_row*ASC16_HEIGHT, ASC16_WIDTH, ASC16_HEIGHT };
    os()->get_screen()->fill_rectangle(rc, m_show_cursor ? CURSOR_COLOR : BACKGROUND_COLOR);
}

void console_t::init()
{
    m_row_num = os()->get_screen()->height() / ASC16_HEIGHT;
    m_col_num = os()->get_screen()->width() / ASC16_WIDTH;
    m_row     = 0;
    m_col     = 0;
    m_tick_to_update = HZ;
    m_show_cursor = true;
    m_lock.init();
    m_input_buffer.init();
    m_wait_queue.init();

    os()->get_dev(DEV_CONSOLE)->read = console_read;
    os()->get_dev(DEV_CONSOLE)->write = console_write;

    draw_background();
    draw_cursor();
}

void console_t::scroll()
{
    if (m_row < m_row_num) {
        return;
    }

    // scroll screen
    os()->get_screen()->scroll();

    // clear last line
    rect_t rc = { 0, (m_row-1)*ASC16_HEIGHT, ASC16_WIDTH*m_col_num, ASC16_HEIGHT };
    os()->get_screen()->fill_rectangle(rc, BACKGROUND_COLOR);

    m_row--;
    m_col = 0;
    draw_cursor();
}

void console_t::put_char(char c, color_ref_t color)
{
    rect_t rc = { m_col*ASC16_WIDTH, m_row*ASC16_HEIGHT, ASC16_WIDTH, ASC16_HEIGHT };
    os()->get_screen()->fill_rectangle(rc, BACKGROUND_COLOR);
    os()->get_screen()->draw_asc16((char) c, m_col*ASC16_WIDTH, m_row*ASC16_HEIGHT, color);
    m_col++;
    if (m_col == m_col_num) {
        m_row++;
        m_col = 0;
        scroll();
    }
    draw_cursor();
}

void console_t::unput_char()
{
    rect_t rc = { m_col*ASC16_WIDTH, m_row*ASC16_HEIGHT, ASC16_WIDTH, ASC16_HEIGHT };
    os()->get_screen()->fill_rectangle(rc, BACKGROUND_COLOR);
}

void console_t::backspace()
{
    unput_char();
    if (m_col == 0) {
        if (m_row == 0) {
            return;
        }
        else {
            m_row--;
            m_col = m_col_num-1;
        }
    }
    else {
        m_col--;
    }
    unput_char();
}

void console_t::putc(int c, color_ref_t color)
{
    uint32 num;
    rect_t rc;

    switch (c) {
        case '\n':
            rc = { m_col*ASC16_WIDTH, m_row*ASC16_HEIGHT, ASC16_WIDTH, ASC16_HEIGHT };
            os()->get_screen()->fill_rectangle(rc, BACKGROUND_COLOR);
            m_row++;
            m_col = 0;
            scroll();
            break;
        case '\t':
            num = (4 - m_col % 4);
            while (num--) {
                put_char(' ', color);
            }
            break;
        case '\b':
            backspace();
            break;
        default:
            put_char((char) c, color);
            break;
    }
    draw_cursor();
}

void console_t::print_int(int32 n, int32 base, int32 sign, color_ref_t color)
{
    static char digits[] = "0123456789abcdef";
    char buffer[16] = {0};

    uint32 num = (uint32)n;
    if (sign && (sign = (n < 0))) {
        num = -n;
    }

    int i = 0;
    do {
        buffer[i++] = digits[num % base];
        num /= base;
    } while (num != 0);

    if (base == 16) {
        while (i < 8) {
            buffer[i++] = '0';
        }
        buffer[i++] = 'x';
        buffer[i++] = '0';
    }

    if (sign) {
        buffer[i++] = '-';
    }

    while (i-- > 0) {
        putc(buffer[i], color);
    }
}

// only support %d %u %x %p %c %s, and seems enough for now
void console_t::kprintf(color_ref_t color, const char *fmt, ...)
{
    static char buffer[BUFFER_SIZE] = {0};
    if (fmt == NULL) {
        return;
    }

    uint32 flags;
    m_lock.lock_irqsave(flags);

    memset(buffer, 0, BUFFER_SIZE);
    va_list ap;
    va_start(ap, fmt);
    int total = vsprintf(buffer, fmt, ap);
    va_end(ap);

    for (int i = 0; i < total; i++) {
        putc(buffer[i], color);
    }
    m_lock.unlock_irqrestore(flags);

#if 0

    uint32 flags;
    m_lock.lock_irqsave(flags);
    if (fmt == NULL) {
        m_lock.unlock_irqrestore(flags);
        return;
    }

    va_list ap;
    va_start(ap, fmt);

    int c = 0;
    char *s = NULL;
    char str_null[] = "NULL";

    for (int i = 0; (c = CHARACTER(fmt[i])) != 0; i++) {
        if (c != '%') {
            putc(c, color);
            continue;
        }

        c = CHARACTER(fmt[++i]);
        if (c == 0) {
            break;
        }

        switch (c) {
            case 'd':
                print_int(va_arg(ap, int32), 10, 1, color);
                break;
            case 'u':
                print_int(va_arg(ap, int32), 10, 0, color);
                break;
            case 'x':
            case 'p':
                print_int(va_arg(ap, int32), 16, 0, color);
                break;
            case 'c':
                putc(va_arg(ap, int32), color);
                break;
            case 's':
                s = va_arg(ap, char *);
                if (s == NULL) {
                    s = str_null;
                }
                for (; *s != '\0'; s++) {
                    putc(*s, color);
                }
                break;
            case '%':
                putc('%', color);
                putc(c, color);
                break;
        }
    }

    va_end(ap);
    m_lock.unlock_irqrestore(flags);
#endif
}

void console_t::draw_time()
{
    uint32 year, month, day, h, m, s;
    rtc_t *rtc = os()->get_arch()->get_rtc();
    year = rtc->year();
    month = rtc->month();
    day = rtc->day();
    h = rtc->hour();
    m = rtc->minute();
    s = rtc->second();
    kprintf(GREEN, "%d-%d-%d %d:%d:%d\n", year, month, day, h, m, s);
}

void console_t::update()
{
    if (--m_tick_to_update != 0) {
        return;
    }

    /* reset tick to update */
    m_tick_to_update = HZ;
    m_show_cursor = !m_show_cursor;
    draw_cursor();
}

void console_t::do_input(char ch)
{
    if (ch != 0) {
        m_tick_to_update = HZ;
        m_show_cursor = true;

        if (ch == '\t') {
            for (int i = 0; i < (4 - m_col % 4); i++) {
                m_input_buffer.input(' ');
            }
        }
        else {
            m_input_buffer.input(ch);
        }
    }
}

int console_t::read(void* buf, int size)
{
    char *p = (char *) buf;
    int left = size;
    while (left > 0) {
        if (m_input_buffer.m_read_index == m_input_buffer.m_write_index) {
            current->sleep_on(&m_wait_queue);
        }
        char c = m_input_buffer.m_buffer[m_input_buffer.m_read_index++ % BUFFER_SIZE];
        *p++ = c;
        --left;
        if (c == '\n') {
            break;
        }
    }

    return size - left;
}

int console_t::write(void* buf, int size)
{
    uint32 flags;
    m_lock.lock_irqsave(flags);
    for (int i = 0; i < size; i++) {
        putc(((char *) buf)[i], WHITE);
    }
    m_lock.unlock_irqrestore(flags);

    return size;
}

void console_t::wakeup_reader()
{
    m_wait_queue.wake_up();
}

