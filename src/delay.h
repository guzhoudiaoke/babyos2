/*
 * guzhoudiaoke@126.com
 * 2018-02-08
 */

#ifndef _DELAY_H_
#define _DELAY_H_

#include "types.h"

class delay_t {
public:
    static void init(uint32 freq);
    static void ms_delay(uint32 ms);
    static void us_delay(uint32 us);

private:
    static void rdtsc_delay(uint64 delta);

private:
    static uint32 s_inited;
    static uint32 s_cpu_freq;
    static uint32 s_cpu_freq_mhz;
};

#endif
