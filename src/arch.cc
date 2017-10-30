/*
 * guzhoudiaoke@126.com
 * 2017-10-26
 */

#include "types.h"
#include "babyos.h"
#include "arch.h"
#include "x86.h"

extern uint32 isr_vector[];

static char* exception_msg[] = {
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

extern "C" {
void isr_0();
void isr_1();
void isr_2();
void isr_3();
void isr_4();
void isr_5();
void isr_6();
void isr_7();
void isr_8();
void isr_9();
void isr_10();
void isr_11();
void isr_12();
void isr_13();
void isr_14();
void isr_15();

void do_irq(trap_frame_t* frame)
{
    os()->get_arch()->get_cpu()->do_irq(frame);
}
}

/*************************************** cpu ******************************************/
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
}

void cpu_t::init_gdt()
{
    console()->kprintf(RED, "cpu_t::init_gdt\n");
    m_gdt[SEG_NULL]  = 0x0000000000000000ULL;
    m_gdt[SEG_KCODE] = 0x00cf9a000000ffffULL;
    m_gdt[SEG_KDATA] = 0x00cf92000000ffffULL;
    m_gdt[SEG_UCODE] = 0x0000000000000000ULL;
    m_gdt[SEG_UDATA] = 0x0000000000000000ULL;

    lgdt(m_gdt, sizeof(m_gdt));
}

void cpu_t::init_idt()
{
    console()->kprintf(RED, "cpu_t::init_idt\n");
    init_isrs();
    lidt(m_idt, sizeof(m_idt));
}

void cpu_t::init()
{
    console()->kprintf(RED, "cpu_t::init()\n");
    init_gdt();
    init_idt();
}

void cpu_t::do_exception(uint32 trapno)
{
    if (trapno <= 0x10) {
        console()->kprintf(RED, "Exception: %s\n", exception_msg[trapno]);
    }
    else {
        console()->kprintf(RED, "Error Interrupt: %x, RESERVED!\n", trapno);
    }
    halt();
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
        os()->get_arch()->get_timer()->do_irq();
        os()->get_arch()->get_rtc()->update();
        os()->get_console()->update();
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

void cpu_t::do_irq(trap_frame_t* frame)
{
    uint32 trapno = frame->trapno;
    if (trapno < IRQ_0) {
        do_exception(trapno);
    }
    else if (trapno < IRQ_0 + 0x10) {
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
    __asm__("nop");
}

/*************************************** 8259a ******************************************/
i8259a_t::i8259a_t()
{
}
i8259a_t::~i8259a_t()
{
}

void i8259a_t::init()
{
    console()->kprintf(RED, "i8259a_t::init()\n");

    /* ICW1：0x11, 表示边沿触发、多片级联、需要发送ICW4 */
    outb(0x20, 0x11);
    outb(0xa0, 0x11);

    /* ICW2：
     * 重新映射 IRQ0~IRQ7 到 0x20~0x27,重新映射 IRQ8~IRQ15 到 0x28~0x2F */
    outb(0x21, 0x20);
    outb(0xa1, 0x28);

    /* ICW3:
     * 	     A0	D7  D6  D5  D4  D3  D2  D1  D0
     * master 1	S7  S6	S5  S4  S3  S2  S1  S0
     * slave  1	0   0   0   0   0   ID2 ID1 ID0
     * master: 0x04，表示主芯片的IR2引脚连接一个从芯片
     * slave: 0x02，表示从片连接到主片的IR2 引脚 */
    outb(0x21, 0x04);
    outb(0xa1, 0x02);

    /* 从片连接到主片的 IRQ2 */
    /* ICW4：0x01，表示8259A芯片被设置成普通嵌套、非中断方式、
     * 用于8086 及其兼容模式 */
    outb(0x21, 0x01);
    outb(0xa1, 0x01);

    /* disable all interrupts */
    outb(0x21, 0xff);
    outb(0xa1, 0xff);
}

void i8259a_t::enable_irq(uint32 irq)
{
	uint8 mask;
	
	if (irq < 8)
	{
		mask = inb(0x21) & (0xff ^ (1 << irq));
		outb(0x21, mask);
	}
	else if (irq < 16)
	{
		mask = inb(0x21) & 0xfb;
		outb(0x21, mask);

		mask = inb(0xa1) & (0xff ^ (1 << (irq-8)));
		outb(0xa1, mask);
	}
}

/*************************************** arch ******************************************/
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
    m_timer.init();
    m_rtc.init();

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

timer_t* arch_t::get_timer()
{
    return &m_timer;
}

rtc_t* arch_t::get_rtc()
{
    return &m_rtc;
}

