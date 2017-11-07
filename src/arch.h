/*
 * guzhoudiaoke@126.com
 * 2017-10-26
 */

#ifndef _ARCH_H_
#define _ARCH_H_

#include "kernel.h"
#include "keyboard.h"
#include "timer.h"
#include "cpu.h"
#include "i8259a.h"

class arch_t {
public:
    arch_t();
    ~arch_t();

    void init();
	void update();

    cpu_t*      get_cpu();
    i8259a_t*   get_8259a();
    keyboard_t* get_keyboard();
    timer_t*    get_timer();
    rtc_t*      get_rtc();

private:
    cpu_t		m_cpu;
    i8259a_t	m_8259a;
    keyboard_t  m_keyboard;
    timer_t     m_timer;
    rtc_t       m_rtc;
};

#endif
