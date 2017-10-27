/*
 * guzhoudiaoke@126.com
 * 2017-10-22
 */

#include "babyos.h"
#include "console.h"
#include "screen.h"
#include "string.h"

extern Screen screen;

Console::Console()
{
}
Console::~Console()
{
}

void Console::draw_background()
{
    rect_t rc = { 0, 0, os()->get_screen()->width(), os()->get_screen()->height() };
    os()->get_screen()->fill_rectangle(rc, BACKGROUND_COLOR);
}

void Console::draw_cursor()
{
    rect_t rc = { m_col*ASC16_WIDTH, m_row*ASC16_HEIGHT, ASC16_WIDTH, ASC16_HEIGHT };
    os()->get_screen()->fill_rectangle(rc, CURSOR_COLOR);
}

void Console::init()
{
    m_row_num = os()->get_screen()->height() / ASC16_HEIGHT;
    m_col_num = os()->get_screen()->width() / ASC16_WIDTH;
    m_row     = 0;
    m_col     = 0;
    memset(m_text, 0, MAX_ROW * MAX_COL);

    draw_background();
    draw_cursor();
}

void Console::put_char(char c, color_ref_t color)
{
    m_text[m_row][m_col] = (char) c;

    rect_t rc = { m_col*ASC16_WIDTH, m_row*ASC16_HEIGHT, ASC16_WIDTH, ASC16_HEIGHT };
    os()->get_screen()->fill_rectangle(rc, BACKGROUND_COLOR);
    os()->get_screen()->draw_asc16((char) c, m_col*ASC16_WIDTH, m_row*ASC16_HEIGHT, color);
    m_col++;
    if (m_col == m_col_num) {
        m_row++;
        m_col = 0;
    }
    draw_cursor();
}

void Console::unput_char()
{
    rect_t rc = { m_col*ASC16_WIDTH, m_row*ASC16_HEIGHT, ASC16_WIDTH, ASC16_HEIGHT };
    os()->get_screen()->fill_rectangle(rc, BACKGROUND_COLOR);
}

void Console::backspace()
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

void Console::putc(int c, color_ref_t color)
{
    uint32 num;
    rect_t rc;

    switch (c) {
        case '\n':
            rc = { m_col*ASC16_WIDTH, m_row*ASC16_HEIGHT, ASC16_WIDTH, ASC16_HEIGHT };
            os()->get_screen()->fill_rectangle(rc, BACKGROUND_COLOR);
            m_row++;
            m_col = 0;
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

void Console::print_int(int32 n, int32 base, int32 sign, color_ref_t color)
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
void Console::kprintf(color_ref_t color, const char *fmt, ...)
{
    if (fmt == NULL) {
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
}
