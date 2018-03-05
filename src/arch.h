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
#include "i8254.h"
#include "io_apic.h"
#include "mp_config.h"
#include "pci.h"
#include "rtl8139.h"

#define MAX_CPU_NUM 8

class arch_t {
public:
    arch_t();
    ~arch_t();

    void init();
	void update();

    cpu_t*      get_cpu(uint32 id);
    cpu_t*      get_boot_processor();
    cpu_t*      get_current_cpu();
    uint32      get_cpu_num();

    i8259a_t*   get_8259a();
    keyboard_t* get_keyboard();
    i8254_t*    get_8254();
    rtc_t*      get_rtc();
    io_apic_t*  get_io_apic();
    pci_t*      get_pci();
    rtl8139_t*  get_rtl8139();

    void        add_processor(uint32 id, uint32 is_bsp);
    void        start_ap();

private:
    uint32      m_cpu_num;
    cpu_t		m_cpu[MAX_CPU_NUM];
    cpu_t*      m_boot_processor;

    i8259a_t	m_8259a;
    keyboard_t  m_keyboard;
    i8254_t     m_8254;
    rtc_t       m_rtc;
    io_apic_t   m_io_apic;
    mp_config_t m_mp_config;
    pci_t       m_pci;
    rtl8139_t   m_rtl8139;
};

#endif
