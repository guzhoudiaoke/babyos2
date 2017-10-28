/*
 * guzhoudiaoke@126.com
 * 2017-10-26
 */

#ifndef _ARCH_H_
#define _ARCH_H_

#include "kernel.h"
#include "keyboard.h"
#include "timer.h"

#define TRAP_GATE_FLAG      (0x00008f0000000000ULL)
#define INTERRUPT_GATE_FLAG (0x00008e0000000000ULL)

#define IRQ_0               (0x20)
#define IRQ_TIMER           (0x0)
#define IRQ_KEYBOARD        (0x1)

typedef struct trap_frame_s {
    uint16 gs;
    uint16 padding1;
    uint16 fs;
    uint16 padding2;
    uint16 es;
    uint16 padding3;
    uint16 ds;
    uint16 padding4;

    // registers as pushed by pusha
    uint32 edi;
    uint32 esi;
    uint32 ebp;
    uint32 oesp;      // useless & ignored
    uint32 ebx;
    uint32 edx;
    uint32 ecx;
    uint32 eax;

    uint32 trapno;

    // below here defined by x86 hardware
    uint32 err;
    uint32 eip;
    uint16 cs;
    uint16 padding5;
    uint32 eflags;

    // below here only when crossing rings, such as from user to kernel
    uint32 esp;
    uint16 ss;
    uint16 padding6;
} trap_frame_t;

class CPU {
public:
    CPU();
    ~CPU();

    void init();
    void do_irq(trap_frame_t* frame);

private:
    void init_gdt();
    void init_idt();
    void init_isrs();

    void set_gate(uint32 index, uint32 addr, uint64 flag);
    void set_trap_gate(uint32 index, uint32 addr);
    void set_intr_gate(uint32 index, uint32 addr);

private:
    uint64	m_gdt[GDT_LEN];
    uint64	m_idt[IDT_LEN];
};

class I8259a {
public:
    I8259a();
    ~I8259a();

    void init();
    void enable_irq(uint32 irq);

private:
};

class Arch {
public:
    Arch();
    ~Arch();

    void init();

    CPU*        get_cpu();
    I8259a*     get_8259a();
    Keyboard*   get_keyboard();
    Timer*      get_timer();
    RTC*        get_rtc();

private:
    CPU		m_cpu;
    I8259a	m_8259a;
    Keyboard m_keyboard;
    Timer   m_timer;
    RTC     m_rtc;
};

#endif
