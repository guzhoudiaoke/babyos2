/*
 * guzhoudiaoke@126.com
 * 2017-11-7
 */

#ifndef _CPU_H_
#define _CPU_H_

#include "kernel.h"
#include "types.h"
#include "syscall.h"
#include "vm.h"
#include "spinlock.h"
#include "list.h"
#include "process.h"
#include "local_apic.h"
#include "tss.h"

#define TRAP_GATE_FLAG      (0x00008f0000000000ULL)
#define INTERRUPT_GATE_FLAG (0x00008e0000000000ULL)
#define SYSTEM_GATE_FLAG    (0x0000ef0000000000ULL)

/* pass args by: eax, edx, ecx.. */
#define FASTCALL(x)	x __attribute__((regparm(2)))

class cpu_t {
public:
    void init(uint32 id, uint32 is_bsp);
    void startup();
    void startup_ap();
    void update();
    void schedule();
    void do_common_isr(trap_frame_t* frame);
    tss_t*        get_tss() { return &m_tss; }
    local_apic_t* get_local_apic();

    /* is boot strap processor */
    uint32  get_apic_id();
    uint32  is_bsp();
    uint32  is_started();
    void    set_is_started(uint32 started);
    uint8*  get_kstack();
    process_t* get_idle();
    void    init_idle();
    void    schedule_tail(process_t* proc);

private:
    void init_gdt();
    void init_idt();
    void init_tss();
    void init_isrs();

    void set_gate(uint32 index, uint32 addr, uint64 flag);
    void set_trap_gate(uint32 index, uint32 addr);
    void set_intr_gate(uint32 index, uint32 addr);
    void set_system_gate(uint32 index, uint32 addr);

    void do_exception(trap_frame_t* frame);
    void do_interrupt(uint32 trapno);
    void do_syscall(trap_frame_t* frame);

private:
    uint32              m_is_bsp;
    uint32              m_apic_id;
    uint64			    m_gdt[GDT_LEN];
    uint64			    m_idt[IDT_LEN];
    tss_t			    m_tss;
    local_apic_t        m_local_apic;
    volatile uint32     m_started;
    //uint8               m_idle[KSTACK_SIZE];
    process_t*          m_idle;
};

#endif
