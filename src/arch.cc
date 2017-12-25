/*
 * guzhoudiaoke@126.com
 * 2017-10-26
 */

#include "types.h"
#include "babyos.h"
#include "arch.h"
#include "x86.h"
#include "string.h"

arch_t::arch_t()
{
}
arch_t::~arch_t()
{
}

void arch_t::init()
{
    m_cpu.init();
    m_8259a.init();
    m_keyboard.init();
    m_8254.init();
    m_rtc.init();
}

void arch_t::update()
{
	m_cpu.update();
	m_rtc.update();
}

cpu_t* arch_t::get_cpu()
{
    return &m_cpu;
}

i8259a_t* arch_t::get_8259a()
{
    return &m_8259a;
}

keyboard_t* arch_t::get_keyboard()
{
    return &m_keyboard;
}

i8254_t* arch_t::get_8254()
{
    return &m_8254;
}

rtc_t* arch_t::get_rtc()
{
    return &m_rtc;
}

