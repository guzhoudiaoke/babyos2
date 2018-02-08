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
    m_cpu_num = 0;
    m_mp_config.init();
    m_8259a.init();
    m_8254.init();
    m_boot_processor->init();
    m_io_apic.init();
    m_keyboard.init();
    m_rtc.init();
}

void arch_t::update()
{
	m_boot_processor->update();
	m_rtc.update();
}

cpu_t* arch_t::get_cpu(uint32 id)
{
    if (id >= m_cpu_num) {
        return NULL;
    }
    return &m_cpu[id];
}

cpu_t* arch_t::get_boot_processor()
{
    return m_boot_processor;
}

void arch_t::add_processor(uint32 id, uint32 is_bsp)
{
    console()->kprintf(CYAN, "add processor: id: %u, is bsp: %u\n", id, is_bsp);
    m_cpu[m_cpu_num].set_id(id);
    m_cpu[m_cpu_num].set_is_bsp(is_bsp);
    if (is_bsp) {
        m_boot_processor = &m_cpu[m_cpu_num];
    }

    m_cpu_num++;
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

io_apic_t* arch_t::get_io_apic()
{
    return &m_io_apic;
}

