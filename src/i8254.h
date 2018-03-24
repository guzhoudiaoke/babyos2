/*
 * guzhoudiaoke@126.com
 * 2018-01-29
 * split from timer.h
 */

#ifndef _i8254_H_
#define _i8254_H_

#include "types.h"

#define HZ              (100)
#define CLOCK_TICK_RATE (1193180)

class i8254_t {
public:
    i8254_t();
    ~i8254_t();

    void init();
    void do_irq();

    uint32 get_timer_count();
    //uint64 get_tick();

private:
    uint64 m_tick;
};


#endif
