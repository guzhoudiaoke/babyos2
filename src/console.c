/*
 * guzhoudiaoke@126.com
 * 2017-10-22
 */

#include "console.h"
#include "screen.h"
#include "string.h"

static console_t console;

static void draw_background()
{
    rect_t rc = { 0, 0, screen_width(), screen_height() };
    fill_rectangle(rc, BACKGROUND_COLOR);
}

void init_console()
{
    console.row_num = screen_height() / ASC16_HEIGHT;
    console.col_num = screen_width() / ASC16_WIDTH;
    console.row     = 0;
    console.col     = 0;
    memset(console.text, 0, MAX_ROW * MAX_COL);

    draw_background();
}

static void console_putchar(char c, color_ref_t color)
{
    console.text[console.row][console.col] = (char) c;
    draw_asc16((char) c, console.col*ASC16_WIDTH, console.row*ASC16_HEIGHT, color);
    console.col++;
    if (console.col == console.col_num) {
        console.row++;
        console.col = 0;
    }
}

static void console_putc(int c, color_ref_t color)
{
    uint32 num;

    switch (c) {
    case '\n':
        console.row++;
        console.col = 0;
        break;
    case '\t':
        num = (4 - console.col % 4);
        while (num--) {
            console_putchar(' ', color);
        }
        break;
    default:
        console_putchar((char) c, color);
        break;
    }
}

static void print_int(int32 n, int32 base, int32 sign, color_ref_t color)
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
        console_putc(buffer[i], color);
    }
}

// only support %d %u %x %p %c %s, and seems enough for now
void kprintf(color_ref_t color, char *fmt, ...)
{
    if (fmt == NULL) {
        return;
    }

    va_list ap;
    va_start(ap, fmt);

    int c = 0;
    char *s = NULL;

    for (int i = 0; (c = CHARACTER(fmt[i])) != 0; i++) {
        if (c != '%') {
            console_putc(c, color);
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
            console_putc(va_arg(ap, int32), color);
            break;
        case 's':
            s = va_arg(ap, char *);
            if (s == NULL) {
                s = "NULL";
            }
            for (; *s != '\0'; s++) {
                console_putc(*s, color);
            }
            break;
        case '%':
            console_putc('%', color);
            console_putc(c, color);
            break;
        }
    }

    va_end(ap);
}
