/*
 * guzhoudiaoke@126.com
 * 2017-11-7
 */

#include "cpu.h"
#include "babyos.h"
#include "i8259a.h"
#include "x86.h"
#include "string.h"
#include "fs.h"

extern uint32 isr_vector[];
extern uint8  kernel_stack[];

static const char* exception_msg[] = {
    "int0  #DE divide error",
    "int1  #DB debug",
    "int2  --  NMI",
    "int3  #BP break point",
    "int4  #OF overflow",
    "int5  #BR bounds check",
    "int6  #UD invalid opcode",
    "int7  #NM device not available",
    "int8  #DF double fault",
    "int9  --  coprocessor seg overrun",
    "int10 #TS invalid TSS",
    "int11 #NP segment not present",
    "int12 #SS stack segment",
    "int13 #GP general protection",
    "int14 #PF page fault",
    "int15 -- （Intel reserved）",
    "int16 #MF x87 FPU coprocessor error",
    "int17 #AC align check",
};

/***************************************************************************/

extern "C" void do_common_isr(trap_frame_t* frame)
{
    os()->get_arch()->get_current_cpu()->do_common_isr(frame);
}

extern "C" void schedule()
{
    os()->get_arch()->get_current_cpu()->schedule();
}

extern "C" void schedule_tail(process_t* proc)
{
    os()->get_arch()->get_current_cpu()->schedule_tail(proc);
}

/* pass args by: eax, edx, ecx.. */
extern "C" void FASTCALL(__switch_to(process_t* prev, process_t* next));
#define switch_to(prev,next,last) do {				\
    asm volatile(									\
        "pushl %%esi\n\t"							\
        "pushl %%edi\n\t"							\
        "pushl %%ebp\n\t"							\
        "movl  %%esp,%0\n\t"	/* save ESP */		\
        "movl  %3,%%esp\n\t"	/* restore ESP */	\
        "movl  $1f,%1\n\t"		/* save EIP */		\
        "pushl %4\n\t"			/* restore EIP */	\
        "movl  %%ebx,%%eax\n\t"                     \
        "jmp __switch_to\n"						    \
        "1:\t"										\
        "popl %%ebp\n\t"							\
        "popl %%edi\n\t"							\
        "popl %%esi\n\t"							\
        :"=m" (prev->m_context.esp),				\
         "=m" (prev->m_context.eip),			    \
         "=b" (last)                                \
        :"m" (next->m_context.esp),				    \
         "m" (next->m_context.eip),				    \
         "d" (next),            				    \
         "b" (prev)                                 \
        :"memory");								    \
} while (0)

extern "C" void __switch_to(process_t* prev, process_t* next)
{
    tss_t* tss = os()->get_arch()->get_current_cpu()->get_tss();
    tss->esp0 = next->m_context.esp0;
}

/***************************************************************************/

void cpu_t::set_gate(uint32 index, uint32 addr, uint64 flag)
{
    uint64 idt_item;

    idt_item = flag;                                            /* gate type */
    idt_item |= (uint64)((SEG_KCODE << 3) << 16);               /* code segment selector */
    idt_item |= ((uint64)addr << 32) & 0xffff000000000000ULL;   /* high 16 bits of address */
    idt_item |= ((uint64)addr) & 0xffff;                        /* low 16 bits of address */

    m_idt[index] = idt_item;
}

/* set idt by trap gate */
void cpu_t::set_trap_gate(uint32 index, uint32 addr)
{
    set_gate(index, addr, TRAP_GATE_FLAG);
}

/* set idt by interrupt gate */
void cpu_t::set_intr_gate(uint32 index, uint32 addr)
{
    set_gate(index, addr, INTERRUPT_GATE_FLAG);
}

void cpu_t::set_system_gate(uint32 index, uint32 addr)
{
    set_gate(index, addr, SYSTEM_GATE_FLAG);
}

void cpu_t::init_isrs()
{
    /* exceptions */
    for (uint32 i = 0; i < IRQ_0; i++) {
        set_trap_gate(i, (uint32)isr_vector[i]);
    }

    /* interrupts */
    for (uint32 i = IRQ_0; i < 256; i++) {
        set_intr_gate(i, (uint32)isr_vector[i]);
    }

    /* syscall */
    set_system_gate(IRQ_SYSCALL, (uint32)isr_vector[IRQ_SYSCALL]);
}

void cpu_t::init_gdt()
{
    m_gdt[SEG_NULL]  = 0x0000000000000000ULL;
    m_gdt[SEG_KCODE] = 0x00cf9a000000ffffULL;
    m_gdt[SEG_KDATA] = 0x00cf92000000ffffULL;
    m_gdt[SEG_UCODE] = 0x00cffa000000ffffULL;
    m_gdt[SEG_UDATA] = 0x00cff2000000ffffULL;

    lgdt(m_gdt, sizeof(uint64) * (GDT_LEN));
}

void cpu_t::init_idt()
{
    init_isrs();
    lidt(m_idt, sizeof(m_idt));

    /*
     * Delete NT
     */
    __asm__("pushfl ; andl $0xffffbfff,(%esp) ; popfl");
}

void cpu_t::init_tss()
{
    memset(&m_tss, 0, sizeof(tss_t));
    m_tss.esp0 = (uint32)&kernel_stack + KSTACK_SIZE;
    m_tss.ss0 = SEG_KDATA << 3;
    m_tss.bitmap = INVALID_IO_BITMAP;
    memset(&m_tss.io_bitmap, ~0, sizeof(uint32) * (IO_BITMAP_SIZE + 1));

    uint64 base = (uint64)&m_tss;
    uint64 limit = (uint64)(sizeof(tss_t) - 1);
    uint64 des = (limit & 0xffff) |					// limit 15-0
        ((base & 0xffffff) << 16) |					// base  23-0
        ((0x9ULL) << 40) |							// type
        ((0x8ULL) << 44) |							// p(1), dlp(00), s(0)
        (((limit >> 16) & 0xf) << 48) |				// limit 19-16
        ((0x00ULL) << 52) |							// g(0), d/b(0), 0, AVL(0)
        (((base) >> 24) & 0xff) << 56;				// base  31-24

    m_gdt[SEG_TSS] = des;
    ltr(SEG_TSS << 3);
}

void cpu_t::init(uint32 apic_id, uint32 is_bsp)
{
    m_is_bsp  = is_bsp;
    m_apic_id = apic_id;
    m_started = 0;
    m_idle = (process_t *) (kernel_stack + m_apic_id*KSTACK_SIZE);
}

void cpu_t::startup()
{
    console()->kprintf(CYAN, "bsp id: %u\n", m_apic_id);
    init_gdt();
    init_idt();
    init_tss();
    init_idle();
    m_local_apic.init();
    m_started = 1;
}

void cpu_t::init_idle()
{
    m_idle->m_pid = 0;
    memset(&m_idle->m_context, 0, sizeof(context_t));
    strcpy(m_idle->m_name, "idle");
    m_idle->m_state = PROCESS_ST_RUNNING;
    m_idle->m_context.esp0 = ((uint32) get_kstack());
    m_idle->m_timeslice = 2;
    m_idle->m_need_resched = 0;
    m_idle->m_sig_queue.init(os()->get_obj_pool_of_size());
    m_idle->m_sig_pending = 0;
    m_idle->m_signal.init();
    m_idle->m_vmm.init();
    m_idle->m_vmm.set_pg_dir(os()->get_mm()->get_kernel_pg_dir());
    m_idle->m_children.init(os()->get_obj_pool_of_size());
    m_idle->m_wait_child.init();
    for (int i = 0; i < MAX_OPEN_FILE; i++) {
        m_idle->m_files[i] = NULL;
    }
}

void cpu_t::startup_ap()
{
    console()->kprintf(GREEN, "startup ap, %u\n", m_apic_id);
    init_gdt();
    init_idt();
    init_tss();
    init_idle();
    m_local_apic.init();
    m_started = 1;
}

void cpu_t::do_exception(trap_frame_t* frame)
{
    uint32 trapno = frame->trapno;
    if (trapno <= 0x10) {
        if (trapno == INT_PF) {
            current->m_vmm.do_page_fault(frame);
            return;
        }

        console()->kprintf(RED, "Exception: %s\n", exception_msg[trapno]);
        console()->kprintf(RED, "current: %p, pid: %p\n", current, current->m_pid);
        console()->kprintf(RED, "errno: %x, eip: %x, cs: %x, esp: %x\n", frame->err, frame->eip, frame->cs, frame->esp);
    }
    else {
        console()->kprintf(RED, "Error Interrupt: %x, RESERVED!\n", trapno);
    }

    while (1) {
        halt();
    }
}

void cpu_t::do_interrupt(uint32 trapno)
{
    if (trapno == IRQ_0 + os()->get_arch()->get_rtl8139()->get_irq()) {
        os()->get_arch()->get_rtl8139()->do_irq();
        m_local_apic.eoi();
        return;
    }

    switch (trapno) {
        case IRQ_0 + IRQ_KEYBOARD:
            os()->get_arch()->get_keyboard()->do_irq();
            m_local_apic.eoi();
            break;
        case IRQ_0 + IRQ_TIMER:
            os()->get_arch()->get_8254()->do_irq();
            break;
        case IRQ_0 + IRQ_HARDDISK:
            os()->get_hd()->do_irq();
            m_local_apic.eoi();
            break;
        case VEC_LOCAL_TIMER:
            m_local_apic.do_timer_irq();
            m_local_apic.eoi();
            break;
        default:
            console()->kprintf(RED, "Interrupt: %x not known.\n", trapno);
            break;
    }
}

void cpu_t::do_syscall(trap_frame_t* frame)
{
    syscall_t::do_syscall(frame);
}

void cpu_t::do_common_isr(trap_frame_t* frame)
{
    uint32 trapno = frame->trapno;
    if (trapno < IRQ_0) {
        do_exception(frame);
    }
    else if (trapno == IRQ_SYSCALL) {
        do_syscall(frame);
    }
    else {
        do_interrupt(trapno);
    }
}

void cpu_t::schedule_tail(process_t* proc)
{
    proc->lock();
    proc->m_has_cpu = 0;
    proc->unlock();
    mb();
}

void cpu_t::schedule()
{
    process_t* prev = current;
    process_t* next = m_idle;

    list_t<process_t *>* run_queue = os()->get_process_mgr()->get_run_queue();
    spinlock_t* rq_lock = run_queue->get_lock();
    uint32 flags;
    rq_lock->lock_irqsave(flags);

    list_t<process_t *>::iterator it = run_queue->begin();
    while (it != run_queue->end()) {
        process_t* p = *it;
        if (p->m_state == PROCESS_ST_RUNNING && p->m_has_cpu == 0) {
            next = p;
            run_queue->erase(it);
            if (next == prev) {
                os()->panic("error next");
            }
            break;
        }
        it++;
    }

    prev->m_need_resched = 0;
    if (next == prev) {
        rq_lock->unlock_irqrestore(flags);
        return;
    }

    if (prev != next && prev->m_pid != 0 && prev->m_state == PROCESS_ST_RUNNING) {
        if (run_queue->find(prev) == run_queue->end()) {
            run_queue->push_back(prev);
        }
    }

    next->m_has_cpu = 1;
    rq_lock->unlock_irqrestore(flags);

    /* switch mm */
    set_cr3(VA2PA(next->m_vmm.get_pg_dir()));

    /* switch registers and stack */
    switch_to(prev, next, prev);

    schedule_tail(prev);
}

void cpu_t::update()
{
    if (--current->m_timeslice == 0) {
        current->m_need_resched = 1;
        current->m_timeslice = 2;
    }
}

local_apic_t* cpu_t::get_local_apic()
{
    return &m_local_apic;
}

uint32 cpu_t::get_apic_id()
{
    return m_apic_id;
}

uint32 cpu_t::is_bsp()
{
    return m_is_bsp;
}

uint32 cpu_t::is_started()
{
    return m_started;
}

void cpu_t::set_is_started(uint32 started)
{
    xchg(&m_started, started);
}

uint8* cpu_t::get_kstack()
{
    return (uint8 *) m_idle + KSTACK_SIZE;
}

process_t* cpu_t::get_idle()
{
    return m_idle;
}
