/*
 * guzhoudiaoke@126.com
 * 2017-10-28
 */

#ifndef _TIMER_H_
#define _TIMER_H_

#include "types.h"

#define HZ              (100)
#define CLOCK_TICK_RATE (1193180)

class Timer {
public:
    Timer();
    ~Timer();

    void init();
    void do_irq();

    uint64 get_tick();

private:
    uint64 m_tick;
};

class RTC {
public:
    RTC();
    ~RTC();

    void init();
    void update();

    uint32 year() { return m_year; }
    uint32 month() { return m_month; }
    uint32 day() { return m_day; }
    uint32 hour() { return m_hour; }
    uint32 minute() { return m_minute; }
    uint32 second() { return m_second; }

private:
    void get_time();
    uint8 bcd_to_binary(uint8 bcd);

private:
    uint8 m_year;
    uint8 m_month;
    uint8 m_day;
    uint8 m_hour;
    uint8 m_minute;
    uint8 m_second;

    uint32 m_tick_to_update;
};

#endif
