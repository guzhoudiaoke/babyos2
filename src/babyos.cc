/*
 * guzhoudiaoke@126.com
 * 2017-10-26
 */

#include "babyos.h"
#include "string.h"

extern BabyOS os;
BabyOS *BabyOS::get_os()
{
    return &os;
}

BabyOS::BabyOS()
{
}

BabyOS::~BabyOS()
{
}

Screen *BabyOS::get_screen() 
{
    return &m_screen; 
}

Console *BabyOS::get_console()
{
    return &m_console;
}

MM* BabyOS::get_mm()
{
    return &m_mm;
}

Arch* BabyOS::get_arch()
{
    return &m_arch;
}

Harddisk* BabyOS::get_harddisk()
{
    return &m_harddisk;
}

void draw_time()
{
    uint32 year, month, day, h, m, s;
    RTC *rtc = os()->get_arch()->get_rtc();
    year = rtc->year();
    month = rtc->month();
    day = rtc->day();
    h = rtc->hour();
    m = rtc->minute();
    s = rtc->second();
    os()->get_console()->kprintf(GREEN, "%d-%d-%d %d:%d:%d\n", year, month, day, h, m, s);
}

IO_clb_t clb;
void test_harddisk()
{
    // only test
    clb.m_flags = 0;
    clb.m_read = 1;
    clb.m_dev = 0;
    clb.m_lba = 244;

    memset(clb.m_buffer, 0, 512);
    os()->get_harddisk()->request(&clb);

    for (int i = 0; i < SECT_SIZE/4; i++) {
        if (i % 8 == 0) {
            os()->get_console()->kprintf(PINK, "\n");
        }
        os()->get_console()->kprintf(PINK, "%x ", ((uint32 *)clb.m_buffer)[i]);
    }
}

void BabyOS::run()
{
    m_screen.init();
    m_console.init();

    m_console.kprintf(WHITE, "Welcome to babyos\n");
    m_console.kprintf(RED,   "Author:\tguzhoudiaoke@126.com\n");

    m_mm.init();
    m_arch.init();
    m_harddisk.init();

    draw_time();
    test_harddisk();

    while (1) {
    }
}

