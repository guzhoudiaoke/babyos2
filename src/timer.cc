/*
 * guzhoudiaoke@126.com
 * 2017-10-28
 */

#include "timer.h"
#include "babyos.h"
#include "arch.h"
#include "x86.h"

Timer::Timer()
{
}
Timer::~Timer()
{
}

void Timer::init()
{
    m_tick = 0;

    uint32 val = CLOCK_TICK_RATE / HZ;

    /* control */
    outb(0x43, 0x36);

    /* clock 0 */
    outb(0x40, uint8(val & 0xff));
    outb(0x40, uint8(val >> 8));

	os()->get_arch()->get_8259a()->enable_irq(IRQ_TIMER);		/* enable keyboard interrupt */
}

void Timer::do_irq()
{
    m_tick++;

    /* EOI */
    outb(0x20, 0x20);
}

uint64 Timer::get_tick()
{
    return m_tick;
}

/***************************************** RTC **********************************************/
RTC::RTC()
{
}
RTC::~RTC()
{
}

void RTC::init()
{
    m_tick_to_update = HZ;
    get_time();
}

void RTC::update()
{
    if (--m_tick_to_update != 0) {
        return;
    }

    m_tick_to_update = HZ;
    m_second++;

    /* every 1 minute, re-read rtc from cmos */
    if (m_second == 60) {
        get_time();
    }
}

void RTC::get_time()
{
    m_year  = bcd_to_binary(cmos_read(9));
    m_month = bcd_to_binary(cmos_read(8));
    m_day   = bcd_to_binary(cmos_read(7));
    m_hour  = bcd_to_binary(cmos_read(4));
    m_minute= bcd_to_binary(cmos_read(2));
    m_second= bcd_to_binary(cmos_read(0));
}

uint8 RTC::bcd_to_binary(uint8 bcd)
{
    return (bcd >> 4) * 10 + (bcd & 0xf);
}

