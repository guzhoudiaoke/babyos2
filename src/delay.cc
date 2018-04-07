/*
 * guzhoudiaoke@126.com
 * 2018-02-08
 */

#include "delay.h"
#include "x86.h"


uint32 delay_t::s_inited = 0;
uint32 delay_t::s_cpu_freq = 0;
uint32 delay_t::s_cpu_freq_mhz = 0;

void delay_t::init(uint32 freq)
{
    if (!s_inited) {
        s_cpu_freq = freq;
        s_cpu_freq_mhz = freq / 1000000;
        s_inited = 1;
    }
}

void delay_t::ms_delay(uint32 ms)
{
    us_delay(ms*1000);
}

void delay_t::us_delay(uint32 us)
{
    rdtsc_delay((uint64) us * s_cpu_freq_mhz);
}

void delay_t::rdtsc_delay(uint64 delta)
{
    uint64 prev, now;
    rdtsc64(prev);
    do {
        nop();
        rdtsc64(now);
    } while (now - prev < delta);
}

