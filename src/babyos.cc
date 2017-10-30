/*
 * guzhoudiaoke@126.com
 * 2017-10-26
 */

#include "babyos.h"
#include "string.h"
#include "x86.h"

extern babyos_t os;
babyos_t* babyos_t::get_os()
{
    return &os;
}

babyos_t::babyos_t()
{
}

babyos_t::~babyos_t()
{
}

screen_t* babyos_t::get_screen() 
{
    return &m_screen; 
}

console_t* babyos_t::get_console()
{
    return &m_console;
}

mm_t* babyos_t::get_mm()
{
    return &m_mm;
}

arch_t* babyos_t::get_arch()
{
    return &m_arch;
}

ide_t* babyos_t::get_ide()
{
    return &m_ide;
}

void draw_time()
{
    uint32 year, month, day, h, m, s;
    rtc_t *rtc = os()->get_arch()->get_rtc();
    year = rtc->year();
    month = rtc->month();
    day = rtc->day();
    h = rtc->hour();
    m = rtc->minute();
    s = rtc->second();
    console()->kprintf(GREEN, "%d-%d-%d %d:%d:%d\n", year, month, day, h, m, s);
}

io_clb_t clb;
void test_ide()
{
    // only test
    clb.m_flags = 0;
    clb.m_read = 1;
    clb.m_dev = 0;
    clb.m_lba = 244;

    memset(clb.m_buffer, 0, 512);
    os()->get_ide()->request(&clb);

    for (int i = 0; i < SECT_SIZE/4; i++) {
        if (i % 8 == 0) {
            console()->kprintf(PINK, "\n");
        }
        console()->kprintf(PINK, "%x ", ((uint32 *)clb.m_buffer)[i]);
    }
}

void babyos_t::run()
{
    m_screen.init();
    m_console.init();

    m_console.kprintf(WHITE, "Welcome to babyos\n");
    m_console.kprintf(RED,   "Author:\tguzhoudiaoke@126.com\n");

    m_mm.init();
    m_arch.init();
    m_ide.init();

    m_console.kprintf(RED, "sti()\n");
    sti();

    draw_time();
    test_ide();

    while (1) {
    }
}

