/*
 * guzhoudiaoke@126.com
 * 2017-10-28
 */

#ifndef _TIMER_H_
#define _TIMER_H_

#include "types.h"

class timer_t {
public:
    timer_t();
    ~timer_t();

    void init(uint64 expires, uint32 data, void (*func)(uint32));
    bool update();

private:
	uint64 m_expires;
	uint32 m_data;
	void (*m_function)(uint32);
};

class rtc_t {
public:
    rtc_t();
    ~rtc_t();

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
