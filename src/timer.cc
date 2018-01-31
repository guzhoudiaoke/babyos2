/*
 * guzhoudiaoke@126.com
 * 2017-10-28
 */

#include "timer.h"
#include "babyos.h"
#include "arch.h"
#include "x86.h"


timer_t::timer_t()
{
}

timer_t::~timer_t()
{
}

void timer_t::init(uint64 expires, uint32 data, void (*func)(uint32))
{
    m_expires = expires;
    m_data = data;
    m_function = func;
}

bool timer_t::update()
{
    if (m_expires == 0) {
        return false;
    }

    --m_expires;
    if (m_expires == 0) {
        m_function(m_data);
        return true;
    }

    return false;
}

/***************************************** rtc_t **********************************************/
rtc_t::rtc_t()
{
}
rtc_t::~rtc_t()
{
}

void rtc_t::init()
{
    m_tick_to_update = HZ;
    get_time();
}

void rtc_t::update()
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

void rtc_t::get_time()
{
    m_year  = bcd_to_binary(cmos_read(9));
    m_month = bcd_to_binary(cmos_read(8));
    m_day   = bcd_to_binary(cmos_read(7));
    m_hour  = bcd_to_binary(cmos_read(4));
    m_minute= bcd_to_binary(cmos_read(2));
    m_second= bcd_to_binary(cmos_read(0));
}

uint8 rtc_t::bcd_to_binary(uint8 bcd)
{
    return (bcd >> 4) * 10 + (bcd & 0xf);
}

