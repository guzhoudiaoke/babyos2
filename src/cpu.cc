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

extern "C" void do_common_isr(trap_frame_t* frame)
{
    os()->get_arch()->get_cpu()->do_common_isr(frame);
}


cpu_t::cpu_t()
{
}
cpu_t::~cpu_t()
{
}

void cpu_t::set_gate(uint32 index, uint32 addr, uint64 flag)
{
    uint64 idt_item;

    idt_item = flag;                                            /* gate type */
    idt_item |= (uint64)((SEG_KCODE << 3) << 16);               /* kernel code segment selector */
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
    // exceptions
    for (uint32 i = 0; i < IRQ_0; i++) {
        set_trap_gate(i, (uint32)isr_vector[i]);
    }

    // interrupts
    for (uint32 i = IRQ_0; i < 256; i++) {
        set_intr_gate(i, (uint32)isr_vector[i]);
    }

    // syscall
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
    m_tss.esp0 = (uint32)&kernel_stack + 2*KSTACK_SIZE;
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
    uint32 *p = (uint32 *)(&des);
    ltr(SEG_TSS << 3);
}

void cpu_t::init_idle_process()
{
    m_idle_process = (process_t *) kernel_stack;
    m_idle_process->m_pid = os()->get_next_pid();

    m_idle_process->m_state = PROCESS_ST_RUNABLE;
    memset(&m_idle_process->m_context, 0, sizeof(context_t));
    strcpy(m_idle_process->m_name, "idle");

    m_idle_process->m_context.esp0 = ((uint32)(&kernel_stack) + 2*KSTACK_SIZE);
    m_idle_process->m_timeslice = 10;
    m_idle_process->m_need_resched = 0;

    // signal
    m_idle_process->m_sig_queue.init();
    m_idle_process->m_sig_pending = 0;
    m_idle_process->m_signal.init();

    // make link
    m_idle_process->m_next = m_idle_process;
    m_idle_process->m_prev = m_idle_process;

    m_idle_process->m_vmm.init();
    m_idle_process->m_vmm.set_pg_dir(os()->get_mm()->get_kernel_pg_dir());
    m_idle_process->m_children.init();
    m_idle_process->m_wait_child.init();
    m_proc_list.push_back(m_idle_process);

    for (int i = 0; i < MAX_OPEN_FILE; i++) {
        m_idle_process->m_files[i] = NULL;
    }

    console()->kprintf(GREEN, "idle: %p, m_tss.esp0: %p, idle->m_contenxt.esp0: %p\n", 
            m_idle_process, m_tss.esp0, m_idle_process->m_context.esp0);
}

void cpu_t::init()
{
    init_gdt();
    init_idt();
    init_tss();

    m_init_process = NULL;
    m_proc_run_queue_lock.init();
    m_timer_list.init();
    m_proc_list.init();

    init_idle_process();
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
    int ch;
    switch (trapno) {
        case IRQ_0 + IRQ_KEYBOARD:
            os()->get_arch()->get_keyboard()->do_irq();
            break;
        case IRQ_0 + IRQ_TIMER:
            os()->get_arch()->get_8254()->do_irq();
            break;
        case IRQ_0 + IRQ_HARDDISK:
            os()->get_ide()->do_irq();
            break;
        default:
            console()->kprintf(RED, "Interrupt: %x\n", trapno);
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
    else if (trapno < IRQ_0 + IRQ_NUM) {
        do_interrupt(trapno);
    }
    else if (trapno == IRQ_SYSCALL) {
        do_syscall(frame);
    }
    else {
        console()->kprintf(RED, "Interrupt: %x, NOT KNOWN\n", frame->trapno);
    }
}

// pass args by: eax, edx, ecx
#define FASTCALL(x)	x __attribute__((regparm(1)))
extern "C" void FASTCALL(__switch_to(process_t* prev));

#define switch_to(prev,next,last) do {					\
    __asm__ volatile(									\
            "pushl %%esi\n\t"							\
            "pushl %%edi\n\t"							\
            "pushl %%ebp\n\t"							\
            "movl  %%esp,%0\n\t"	/* save ESP */		\
            "movl  %2,%%esp\n\t"	/* restore ESP */	\
            "movl  $1f,%1\n\t"		/* save EIP */		\
            "pushl %3\n\t"			/* restore EIP */	\
            "jmp __switch_to\n"						\
            "1:\t"										\
            "popl %%ebp\n\t"							\
            "popl %%edi\n\t"							\
            "popl %%esi\n\t"							\
            :"=m" (prev->m_context.esp),				\
            "=m" (prev->m_context.eip)				\
            :"m" (next->m_context.esp),				\
            "m" (next->m_context.eip),				\
            "a" (next)            					\
            );													\
} while (0)

extern "C"
void __switch_to(process_t* next)
{
    tss_t* tss = os()->get_arch()->get_cpu()->get_tss();
    tss->esp0 = next->m_context.esp0;
}

void cpu_t::schedule()
{
    cli();
    process_t* prev = current;
    process_t* next = current->m_next;
    while (next != prev && next->m_state != PROCESS_ST_RUNABLE) {
        next = next->m_next;
    }
    if (prev->m_state != PROCESS_ST_RUNABLE) {
        remove_process_from_list(prev);
    }
    sti();

    if (prev == next) {
        return;
    }

    set_cr3(VA2PA(next->m_vmm.get_pg_dir()));

    prev->m_need_resched = 0;
    switch_to(prev, next, prev);
}

extern "C"
void schedule()
{
    os()->get_arch()->get_cpu()->schedule();
}

bool cpu_t::is_in_process_list(process_t* proc)
{
    bool find = false;
    m_proc_run_queue_lock.lock();
    process_t* p = m_idle_process->m_next;
    while (p != m_idle_process) {
        if (p == proc) {
            find = true;
            break;
        }
        p = p->m_next;
    }
    m_proc_run_queue_lock.unlock();
    return find;
}

void cpu_t::add_process_to_list(process_t* proc)
{
    m_proc_run_queue_lock.lock();
    proc->m_next = m_idle_process;
    proc->m_prev = m_idle_process->m_prev;
    m_idle_process->m_prev->m_next = proc;
    m_idle_process->m_prev = proc;
    m_proc_run_queue_lock.unlock();
}

void cpu_t::remove_process_from_list(process_t* proc)
{
    m_proc_run_queue_lock.lock();
    proc->m_prev->m_next = proc->m_next;
    proc->m_next->m_prev = proc->m_prev;
    m_proc_run_queue_lock.unlock();
}

process_t* cpu_t::fork(trap_frame_t* frame)
{
    // fork a new process
    process_t*p = current->fork(frame);

    // close interrupt
    cli();

    // link to list
    add_process_to_list(p);

    // add a child for current
    current->m_children.push_back(p);

    m_proc_list.push_back(p);

    // start interrupt
    sti();

    return p;
}

void cpu_t::update()
{
    if (--current->m_timeslice == 0) {
        current->m_need_resched = 1;
        current->m_timeslice = 10;
    }

    list_t<timer_t*>::iterator it = m_timer_list.begin();
    while (it != m_timer_list.end()) {
        (*it)->update();
        it++;
    }
}

void cpu_t::add_timer(timer_t* timer)
{
    cli();
    m_timer_list.push_back(timer);
    sti();
}

void cpu_t::remove_timer(timer_t* timer)
{
    cli();
    list_t<timer_t*>::iterator it = m_timer_list.begin();
    while (it != m_timer_list.end()) {
        if (timer == *it) {
            m_timer_list.erase(it);
            break;
        }
    }
    sti();
}

void cpu_t::wake_up_process(process_t* proc)
{
    cli();
    proc->wake_up();
    if (!is_in_process_list(proc)) {
        add_process_to_list(proc);
    }
    sti();
}

process_t* cpu_t::get_child_reaper()
{
    // if have not set init process, set it by idle process's child
    if (m_init_process == NULL) {
        m_init_process = *m_idle_process->m_children.begin();
    }

    return m_init_process;
}

process_t* cpu_t::get_idle()
{
    return m_idle_process;
}

void cpu_t::release_process(process_t* proc)
{
    list_t<process_t*>::iterator it = m_proc_list.begin();
    while (it != m_proc_list.end()) {
        if (*it == proc) {
            m_proc_list.erase(it);
            break;
        }
        it++;
    }

    os()->get_mm()->free_pages(proc, 1);
}

process_t* cpu_t::find_process(uint32 pid)
{
    process_t* p = NULL;
    list_t<process_t*>::iterator it = m_proc_list.begin();
    while (it != m_proc_list.end()) {
        if ((*it)->m_pid == pid) {
            p = *it;
            break;
        }
        it++;
    }

    return p;
}

int32 cpu_t::send_signal_to(uint32 pid, uint32 sig)
{
    console()->kprintf(WHITE, "send_signal, ");

    siginfo_t si;
    si.m_sig = sig;
    si.m_pid = current->m_pid;

    cli();
    process_t* p = find_process(pid);
    if (p != NULL) {
        p->m_sig_queue.push_back(si);
        p->calc_sig_pending();
    }
    sti();

    return 0;
}

extern "C"
void do_signal(trap_frame_t* frame)
{
    os()->get_arch()->get_cpu()->do_signal(frame);
}

void cpu_t::do_signal(trap_frame_t* frame)
{
    if (!current->m_sig_queue.empty()) {
        siginfo_t si = *current->m_sig_queue.begin();
        current->m_sig_queue.pop_front();
        current->calc_sig_pending();
        current->m_signal.handle_signal(frame, si);
    }
}

