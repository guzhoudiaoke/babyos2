/*
 * guzhoudiaoke@126.com
 * 2017-10-26
 */

#ifndef _ARCH_H_
#define _ARCH_H_

#include "kernel.h"
#include "keyboard.h"
#include "timer.h"
#include "systemcall.h"

#define TRAP_GATE_FLAG      (0x00008f0000000000ULL)
#define INTERRUPT_GATE_FLAG (0x00008e0000000000ULL)

#define IRQ_0               (0x20)
#define IRQ_NUM             (0x10)
#define IRQ_TIMER           (0x0)
#define IRQ_KEYBOARD        (0x1)
#define IRQ_HARDDISK        (0xe)
#define IRQ_SYSCALL         (0x80)

class cpu_t {
public:
    cpu_t();
    ~cpu_t();

    void init();
    void do_irq(trap_frame_t* frame);
    void sleep();

private:
    void init_gdt();
    void init_idt();
    void init_isrs();

    void set_gate(uint32 index, uint32 addr, uint64 flag);
    void set_trap_gate(uint32 index, uint32 addr);
    void set_intr_gate(uint32 index, uint32 addr);

    void do_exception(uint32 trapno);
    void do_interrupt(uint32 trapno);
    void do_syscall(trap_frame_t* frame);

private:
    uint64	m_gdt[GDT_LEN];
    uint64	m_idt[IDT_LEN];
    systemcall_t m_syscall;
};

class i8259a_t {
public:
    i8259a_t();
    ~i8259a_t();

    void init();
    void enable_irq(uint32 irq);

private:
};

class arch_t {
public:
    arch_t();
    ~arch_t();

    void init();

    cpu_t*      get_cpu();
    i8259a_t*   get_8259a();
    keyboard_t* get_keyboard();
    timer_t*    get_timer();
    rtc_t*      get_rtc();

private:
    cpu_t		m_cpu;
    i8259a_t	m_8259a;
    keyboard_t  m_keyboard;
    timer_t     m_timer;
    rtc_t       m_rtc;
};

#endif
