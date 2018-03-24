/*
 * guzhoudiaoke@126.com
 * 2018-02-07
 */

#ifndef _TIMER_MGR_H_
#define _TIMER_MGR_H_

#include "types.h"
#include "list.h"
#include "timer.h"

class timer_mgr_t {
public:
    void init();
    void update();
    void add_timer(timer_t* timer);
    void remove_timer(timer_t* timer);

private:
    list_t<timer_t *>    m_timer_list;
};

#endif
