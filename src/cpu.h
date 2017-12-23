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


#define TRAP_GATE_FLAG      (0x00008f0000000000ULL)
#define INTERRUPT_GATE_FLAG (0x00008e0000000000ULL)
#define SYSTEM_GATE_FLAG    (0x0000ef0000000000ULL)

#define IO_BITMAP_SIZE		(32)
#define INVALID_IO_BITMAP	(0x8000)


#define INT_PF				(14)


struct process_s;
typedef struct process_s process_t;
static inline process_t* get_current(void)
{
    process_t* current;
    __asm__("andl %%esp,%0; ":"=r" (current) : "0" (~8191UL));

    return current;
}

#define current get_current()


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

typedef struct context_s {
    uint32	esp0;
    uint32	eip;
    uint32	esp;
    uint32	fs;
    uint32	gs;
} context_t;

enum {
    PROCESS_ST_PREPARE = 0,
    PROCESS_ST_RUNABLE,
    PROCESS_ST_RUNNING,
    PROCESS_ST_SLEEP,
    PROCESS_ST_ZOMBIE,
};

typedef struct process_s {
    uint32      m_need_resched;
    char		m_name[32];
    pid_t		m_pid;
    uint32		m_state;
    context_t	m_context;
    vmm_t		m_vmm;
    uint32      m_timeslice;

    process_t*	m_prev;
    process_t*	m_next;
} process_t;

class cpu_t {
public:
    cpu_t();
    ~cpu_t();

    void init();
    void do_common_isr(trap_frame_t* frame);
    void sleep();
    void schedule();
    tss_t* get_tss() { return &m_tss; }
    process_t* fork(trap_frame_t* frame);
    void update();

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

    void add_process(process_t* proc);

private:
    uint64			m_gdt[GDT_LEN];
    uint64			m_idt[IDT_LEN];
    tss_t			m_tss;
    process_t*		m_idle_process;
    uint32          m_next_pid;
    spinlock_t      m_proc_list_lock;
};

#endif
