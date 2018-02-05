/*
 * guzhoudiaoke@126.com
 * 2018-01-25
 */

#ifndef _LOCAL_APIC_H_
#define _LOCAL_APIC_H_

#include "types.h"


#define APIC_BASE       (0xfee00000)


class local_apic_t {
public:
    int  check();
    void global_enable();
    void global_disable();
    void software_enable();
    void software_disable();

    uint32 calibrate_clock();
    void do_timer_irq();
    void eoi();

    void test();
    int  init_timer();
    int  init();

private:
    uint64 m_tick;
};

#endif
