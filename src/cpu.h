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
#include "timer.h"
#include "list.h"
#include "process.h"
#include "local_apic.h"


#define TRAP_GATE_FLAG      (0x00008f0000000000ULL)
#define INTERRUPT_GATE_FLAG (0x00008e0000000000ULL)
#define SYSTEM_GATE_FLAG    (0x0000ef0000000000ULL)

#define IO_BITMAP_SIZE		(32)
#define INVALID_IO_BITMAP	(0x8000)


#define INT_PF				(14)


/* tss struct defined in linux */
typedef struct tss_s {
    uint16	back_link,__blh;
    uint32	esp0;
    uint16	ss0,__ss0h;
    uint32	esp1;
    uint16	ss1,__ss1h;
    uint32	esp2;
    uint16	ss2,__ss2h;
    uint32  __cr3;
    uint32  eip;
    uint32  eflags;
    uint32  eax,ecx,edx,ebx;
    uint32  esp;
    uint32  ebp;
    uint32  esi;
    uint32  edi;
    uint16	es, __esh;
    uint16	cs, __csh;
    uint16	ss, __ssh;
    uint16	ds, __dsh;
    uint16	fs, __fsh;
    uint16	gs, __gsh;
    uint16	ldt, __ldth;
    uint16	trace, bitmap;
    uint32	io_bitmap[IO_BITMAP_SIZE+1];
    /*
     * pads the TSS to be cacheline-aligned (size is 0x100)
     */
    uint32 __cacheline_filler[5];
} tss_t;

class cpu_t {
public:
    cpu_t();
    ~cpu_t();

    void init();
    void do_common_isr(trap_frame_t* frame);
    void schedule();
    tss_t* get_tss() { return &m_tss; }
    process_t* fork(trap_frame_t* frame);
    void update();
    void wake_up_process(process_t* proc);
    process_t* get_child_reaper();
    process_t* get_idle();
    void release_process(process_t* proc);

    int32 send_signal_to(uint32 pid, uint32 sig);
    void do_signal(trap_frame_t* frame);

    local_apic_t* get_local_apic();

private:
    void init_gdt();
    void init_idt();
    void init_tss();
    void init_isrs();
    void init_idle_process();

    void set_gate(uint32 index, uint32 addr, uint64 flag);
    void set_trap_gate(uint32 index, uint32 addr);
    void set_intr_gate(uint32 index, uint32 addr);
    void set_system_gate(uint32 index, uint32 addr);

    void do_exception(trap_frame_t* frame);
    void do_interrupt(uint32 trapno);
    void do_syscall(trap_frame_t* frame);

    bool is_in_run_queue(process_t* proc);
    void add_process_to_list(process_t* proc);
    void remove_process_from_list(process_t* proc);

    process_t* find_process(uint32 pid);

private:
    uint64			    m_gdt[GDT_LEN];
    uint64			    m_idt[IDT_LEN];
    tss_t			    m_tss;
    process_t*		    m_idle_process;
    process_t*		    m_init_process;
    spinlock_t          m_rq_lock;
    list_t<process_t*>  m_proc_list;
    local_apic_t        m_local_apic;
};

#endif
