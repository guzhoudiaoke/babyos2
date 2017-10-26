/*
 * guzhoudiaoke@126.com
 * 2017-10-26
 */

#include "types.h"
#include "babyos.h"
#include "arch.h"
#include "x86.h"

extern uint32 isr_vec[];
uint32 isr_table[NR_IRQ];

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

extern "C" void int_0_divide_error();
extern "C" void int_1_debug();
extern "C" void int_2_nmi();
extern "C" void int_3_break_point();
extern "C" void int_4_overflow();
extern "C" void int_5_bounds_check();
extern "C" void int_6_invalid_opcode();
extern "C" void int_7_device_not_available();
extern "C" void int_8_double_fault();
extern "C" void int_9_coprocessor_seg_overrun();
extern "C" void int_10_invalid_tss();
extern "C" void int_11_segment_not_present();
extern "C" void int_12_stack_segment();
extern "C" void int_13_general_protection();
extern "C" void int_14_page_fault();
extern "C" void int_15_reserved();
extern "C" void int_16_coprocessor_error();

extern "C" void isr_0();
extern "C" void isr_1();
extern "C" void isr_2();
extern "C" void isr_3();
extern "C" void isr_4();
extern "C" void isr_5();
extern "C" void isr_6();
extern "C" void isr_7();
extern "C" void isr_8();
extern "C" void isr_9();
extern "C" void isr_10();
extern "C" void isr_11();
extern "C" void isr_12();
extern "C" void isr_13();
extern "C" void isr_14();
extern "C" void isr_15();


extern "C" 
void do_isr(trap_frame_t* frame)
{
    os()->get_console()->kprintf(GREEN, "do_isr\n");
    return;
    os()->get_arch()->get_cpu()->do_isr(frame);
}

/*************************************** cpu ******************************************/
CPU::CPU()
{
}
CPU::~CPU()
{
}

void CPU::set_gate(uint32 index, uint32 addr, uint64 flag)
{
	uint64 idt_item;

	idt_item = flag;                                            /* gate type */
	idt_item |= (uint64)((SEG_KCODE << 3) << 16);               /* kernel code segment selector */
	idt_item |= ((uint64)addr << 32) & 0xffff000000000000ULL;   /* high 16 bits of address */
	idt_item |= ((uint64)addr) & 0xffff;                        /* low 16 bits of address */

	m_idt[index] = idt_item;
    //os()->get_console()->kprintf(GREEN, "%x-%x\t", (uint32)m_idt[index], (uint32) (m_idt[index]>>32));
}

/* set idt by trap gate */
void CPU::set_trap_gate(uint32 index, uint32 addr)
{
	set_gate(index, addr, TRAP_GATE_FLAG);
}

/* set idt by interrupt gate */
void CPU::set_interrupt_gate(uint32 index, uint32 addr)
{
	set_gate(index, addr, INTERRUPT_GATE_FLAG);
}

void CPU::init_traps()
{
/*	for (int i = 0; i < 32; i++) {
		set_trap_gate(i, (uint32)&isr_vec[0]);
        //os()->get_console()->kprintf(GREEN, "%x\t", (uint32)&isr_vec[i]);
	}*/
 	set_trap_gate(0,  (uint32)&int_0_divide_error);
 	set_trap_gate(1,  (uint32)&int_1_debug); 
 	set_trap_gate(2,  (uint32)&int_2_nmi); 
 	set_trap_gate(3,  (uint32)&int_3_break_point); 
 	set_trap_gate(4,  (uint32)&int_4_overflow);
 	set_trap_gate(5,  (uint32)&int_5_bounds_check); 
 	set_trap_gate(6,  (uint32)&int_6_invalid_opcode); 
 	set_trap_gate(7,  (uint32)&int_7_device_not_available);
 	set_trap_gate(8,  (uint32)&int_8_double_fault);
 	set_trap_gate(9,  (uint32)&int_9_coprocessor_seg_overrun); 
 	set_trap_gate(10, (uint32)&int_10_invalid_tss);
 	set_trap_gate(11, (uint32)&int_11_segment_not_present); 
 	set_trap_gate(12, (uint32)&int_12_stack_segment);
 	set_trap_gate(13, (uint32)&int_13_general_protection);
 	set_trap_gate(14, (uint32)&int_14_page_fault);
 	set_trap_gate(15, (uint32)&int_15_reserved);
 	set_trap_gate(16, (uint32)&int_16_coprocessor_error);

	for (int i = 17; i < 32; i++) {
		set_trap_gate(i, (uint32)&int_15_reserved);
    }
}

void CPU::init_isrs()
{
    /*for (uint32 i = 32; i < 48; i++) {
        //set_interrupt_gate(i, (uint32)&isr_vec[i]);
		set_trap_gate(i, (uint32)&isr_vec[0]);
        //os()->get_console()->kprintf(GREEN, "%x\t", (uint32)&isr_vec[i]);
    }*/

	set_interrupt_gate(32, (uint32)&isr_0);
	set_interrupt_gate(33, (uint32)&isr_1);
	set_interrupt_gate(34, (uint32)&isr_2);
	set_interrupt_gate(35, (uint32)&isr_3);
	set_interrupt_gate(36, (uint32)&isr_4);
	set_interrupt_gate(37, (uint32)&isr_5);
	set_interrupt_gate(38, (uint32)&isr_6);
	set_interrupt_gate(39, (uint32)&isr_7);
	set_interrupt_gate(40, (uint32)&isr_8);
	set_interrupt_gate(41, (uint32)&isr_9);
	set_interrupt_gate(42, (uint32)&isr_10);
	set_interrupt_gate(43, (uint32)&isr_11);
	set_interrupt_gate(44, (uint32)&isr_12);
	set_interrupt_gate(45, (uint32)&isr_13);
	set_interrupt_gate(46, (uint32)&isr_14);
	set_interrupt_gate(47, (uint32)&isr_15);
}

void CPU::init_gdt()
{
    m_gdt[SEG_NULL]  = 0x0000000000000000ULL;
    m_gdt[SEG_KCODE] = 0x00cf9a000000ffffULL;
    m_gdt[SEG_KDATA] = 0x00cf92000000ffffULL;
    m_gdt[SEG_UCODE] = 0x0000000000000000ULL;
    m_gdt[SEG_UDATA] = 0x0000000000000000ULL;

    lgdt(m_gdt, sizeof(m_gdt));
}

void CPU::init_idt()
{
    init_traps();
    init_isrs();
    lidt(m_idt, sizeof(m_idt));
}

void CPU::init()
{
    init_gdt();
    init_idt();
}

void CPU::do_isr(trap_frame_t* frame)
{
    if (frame->trapno <= 17) {
        os()->get_console()->kprintf(RED, "Exception: %s\n", exception_msg[frame->trapno]);
    }
    else if (frame->trapno <= 32) {
        os()->get_console()->kprintf(RED, "Interrupt: %x, RESERVED\n", frame->trapno);
    }
    else if (frame->trapno < 48) {
        os()->get_console()->kprintf(RED, "Interrupt: %x\n", frame->trapno);
    }
    else {
        os()->get_console()->kprintf(RED, "Interrupt: %x, NOT KNOWN\n", frame->trapno);
    }
    halt();
}

/*************************************** 8259a ******************************************/
I8259a::I8259a()
{
}
I8259a::~I8259a()
{
}

void I8259a::init()
{
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
	
	/* 关闭 IRQ0~IRQ7 号中断 */
	outb(0x21, 0xff);
	/* 关闭 IRQ8~IRQ15 号中断 */
	outb(0xa1, 0xff);
}

/*************************************** arch ******************************************/
Arch::Arch()
{
}
Arch::~Arch()
{
}

void Arch::init()
{
	m_cpu.init();
	m_8259a.init();
    sti();
}

CPU* Arch::get_cpu()
{
    return &m_cpu;
}

