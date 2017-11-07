/*
 * guzhoudiaoke@126.com
 * 2017-11-7
 */

#include "cpu.h"
#include "babyos.h"
#include "i8259a.h"
#include "x86.h"
#include "string.h"

extern uint32 isr_vector[];
extern uint8  kernel_stack[];
extern void ret_from_isr(void) __asm__("ret_from_isr");

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
    m_next_pid = 0;
    m_idle_process = (process_t *) kernel_stack;

    m_idle_process->m_pid = m_next_pid++;
    m_idle_process->m_state = PROCESS_ST_RUNABLE;
    memset(&m_idle_process->m_context, 0, sizeof(context_t));
    strcpy(m_idle_process->m_name, "idle");

    m_idle_process->m_pg_dir = os()->get_mm()->get_pg_dir();
    m_idle_process->m_context.esp0 = ((uint32)(&kernel_stack) + 2*KSTACK_SIZE);
    m_idle_process->m_need_resched = 1;

    // make link
    m_idle_process->m_next = m_idle_process;
    m_idle_process->m_prev = m_idle_process;

	console()->kprintf(GREEN, "idle: %p, m_tss.esp0: %p, idle->m_contenxt.esp0: %p\n", 
		m_idle_process, m_tss.esp0, m_idle_process->m_context.esp0);
}

void cpu_t::init()
{
    init_gdt();
    init_idt();
    init_tss();
    init_idle_process();
}

void cpu_t::do_exception(trap_frame_t* frame)
{
    uint32 trapno = frame->trapno;
    if (trapno <= 0x10) {
        console()->kprintf(RED, "Exception: %s\n", exception_msg[trapno]);
        console()->kprintf(RED, "current: %p, pid: %p\n", current, current->m_pid);
        console()->kprintf(RED, "errno: %x, eip: %x, cs: %x, esp: %x\n", frame->err, frame->eip, frame->cs, frame->esp);

        uint32 cr2 = 0xffffffff;
        __asm__ volatile("movl %%cr2, %%eax" : "=a" (cr2));
        console()->kprintf(RED, "cr2: %x\n", cr2);
    }
    else {
        console()->kprintf(RED, "Error Interrupt: %x, RESERVED!\n", trapno);
    }
    while (1) {
        halt();
    }
}

void test()
{
}

void cpu_t::do_interrupt(uint32 trapno)
{
    int ch;
    switch (trapno) {
        case IRQ_0 + IRQ_KEYBOARD:
            os()->get_arch()->get_keyboard()->do_irq();
            ch = os()->get_arch()->get_keyboard()->read();
            if (ch != 0) {
                console()->kprintf(WHITE, "%c", ch);
            }
            break;
        case IRQ_0 + IRQ_TIMER:
			os()->update(os()->get_arch()->get_timer()->get_tick());
            os()->get_arch()->get_timer()->do_irq();
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
    m_syscall.do_syscall(frame);
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

void cpu_t::sleep()
{
    // FIXME: only test
    __asm__("nop");
}

// pass args by: eax, edx, ecx
#define FASTCALL(x)	x __attribute__((regparm(1)))
extern "C" void FASTCALL(__switch_to(process_t* prev));

//#define switch_to(prev,next,last) do {					\
//	__asm__ volatile(									\
//			 "pushl %%esi\n\t"							\
//		     "pushl %%edi\n\t"							\
//		     "pushl %%ebp\n\t"							\
//		     "movl  %%esp,%0\n\t"	/* save ESP */		\
//		     "movl  %3,%%esp\n\t"	/* restore ESP */	\
//		     "movl  $1f,%1\n\t"		/* save EIP */		\
//		     "pushl %4\n\t"			/* restore EIP */	\
//		     "jmp __switch_to\n"						\
//		     "1:\t"										\
//		     "popl %%ebp\n\t"							\
//		     "popl %%edi\n\t"							\
//		     "popl %%esi\n\t"							\
//		     :"=m" (prev->m_context.esp),				\
//			  "=m" (prev->m_context.eip),				\
//		      "=b" (last)								\
//		     :"m" (next->m_context.esp),				\
//			  "m" (next->m_context.eip),				\
//		      "a" (prev), "d" (next)					\
//	);													\
//} while (0)

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

extern "C" void __switch_to(process_t* next)
{
    tss_t* tss = os()->get_arch()->get_cpu()->get_tss();
    tss->esp0 = next->m_context.esp0;
}

void cpu_t::schedule()
{
	if ((uint32)current % 8192 != 0) {
		console()->kprintf(RED, "ERROR current: %p\n", current);
	}

    process_t* prev = current;
    process_t* next = current->m_next;
    if (prev == next) {
        return;
    }

    //while (1) {
    //    next = next->m_next;
    //    if (next == current) {
    //        return;
    //    }
    //    if (next->m_state == PROCESS_ST_RUNABLE) {
    //        break;
    //    }
    //}
    prev->m_need_resched = 0;
    switch_to(prev, next, prev);
}

    extern "C"
void schedule()
{
    os()->get_arch()->get_cpu()->schedule();
}

process_t* cpu_t::fork(trap_frame_t* frame)
{
    // alloc a process_t
    process_t* p = (process_t *)os()->get_mm()->boot_mem_alloc(PAGE_SIZE*2, 1);
    *p = *current;

    // kstack
    trap_frame_t* child_frame = ((trap_frame_t *) ((uint32(p) + PAGE_SIZE*2))) - 1;
    memcpy(child_frame, frame, sizeof(trap_frame_t));
    child_frame->eax = 0;

    // pg_dir

    // context
    p->m_context.esp = (uint32) child_frame;
    p->m_context.esp0 = (uint32) (child_frame + 1);
    p->m_context.eip = (uint32) ret_from_isr;

    // pid, need check if same with other process
    p->m_pid = m_next_pid++;

    // change state
    p->m_state = PROCESS_ST_RUNABLE;
    p->m_need_resched = 1;

    // link to list
    p->m_next = m_idle_process;
    p->m_prev = m_idle_process->m_prev;
    m_idle_process->m_prev->m_next = p;
    m_idle_process->m_prev = p;

    return p;
}

void cpu_t::update()
{
	current->m_need_resched = 1;
}

