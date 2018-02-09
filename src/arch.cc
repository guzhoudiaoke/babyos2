/*
 * guzhoudiaoke@126.com
 * 2017-10-26
 */

#include "types.h"
#include "babyos.h"
#include "arch.h"
#include "x86.h"
#include "string.h"
#include "delay.h"

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
    m_boot_processor->startup();
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
    console()->kprintf(CYAN, "add processor: id: %u, is bsp: %u\n", id, is_bsp ? 1 : 0);
    m_cpu[m_cpu_num].init(id, is_bsp);
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

extern "C" int apmain(void);
extern pde_t entry_pg_dir[];
extern uint32 _binary_start_ap_start[], _binary_start_ap_size[];

void arch_t::start_ap()
{
    uint32* ap_start_addr = (uint32 *) PA2VA(AP_START_ADDR);
    uint32* ap_main       = (uint32 *) PA2VA(AP_MAIN);
    uint32* ap_kstack     = (uint32 *) PA2VA(AP_KSTACK);
    uint32* ap_pgdir      = (uint32 *) PA2VA(AP_PGDIR);
    uint32* ap_index      = (uint32 *) PA2VA(AP_INDEX);

    *ap_main  = (uint32) apmain;
    *ap_pgdir = (uint32) VA2PA(entry_pg_dir);
    //*ap_pgdir = (uint32) VA2PA(os()->get_mm()->get_kernel_pg_dir());
    memcpy(ap_start_addr, _binary_start_ap_start, (uint32) _binary_start_ap_size);

    //console()->kprintf(PINK, "start_ap: %p, size: %x\n", (uint32) _binary_start_ap_start, 
    //        (uint32) _binary_start_ap_size);
    console()->kprintf(PINK, "ap_main: %p, %p, %p, %p\n", AP_MAIN, ap_main, *ap_main, apmain);
    console()->kprintf(PINK, "ap_pgdir: %p, %p, %p\n", AP_PGDIR, ap_pgdir, *ap_pgdir);
            
    //for (int i = 0; i < (uint32) _binary_start_ap_size / 4; i++) {
    //    if (i % 8 == 0) {
    //        console()->kprintf(PINK, "\n");
    //    }
    //    console()->kprintf(PINK, "%x, ", ap_start_addr[i]);
    //}
    //console()->kprintf(PINK, "\n");

    for (int i = 0; i < m_cpu_num; i++) {
        if (m_cpu[i].is_bsp()) {
            continue;
        }
        
        *ap_index = i;
        *ap_kstack = (uint32) m_cpu[i].get_kstack() + KSTACK_SIZE;
        //*ap_kstack = (uint32) os()->get_mm()->alloc_pages(1) + KSTACK_SIZE;
        console()->kprintf(PINK, "ap_kstack: %p, %p, %p\n", AP_KSTACK, ap_kstack, *ap_kstack);

        m_cpu[i].get_local_apic()->start_ap(m_cpu[i].get_apic_id(), AP_START_ADDR);

        console()->kprintf(YELLOW, "wait ap started\n");
        while (!m_cpu[i].is_started()) {
            delay_t::ms_delay(1000);
            console()->kprintf(CYAN, "check cpu %u started: %u\n", i, m_cpu[i].is_started());
        }
        console()->kprintf(YELLOW, "CPU %u started.\n", m_cpu[i].get_apic_id());
    }
}

cpu_t* arch_t::get_current_cpu()
{
    uint32 flags;
    cpu_t* cpu = NULL;
    local_irq_save(flags);
    uint32 id = local_apic_t::get_apic_id();
    for (int i = 0; i < m_cpu_num; i++) {
        if (m_cpu[i].get_apic_id() == id) {
            cpu = &m_cpu[i];
            break;
        }
    }
    local_irq_restore(flags);

    return cpu;
}
