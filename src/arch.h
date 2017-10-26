/*
 * guzhoudiaoke@126.com
 * 2017-10-26
 */

#ifndef _ARCH_H_
#define _ARCH_H_

#include "kernel.h"

#define NR_IRQ	32

#define TRAP_GATE_FLAG      0x00008f0000000000ULL
#define INTERRUPT_GATE_FLAG 0x00008e0000000000ULL

typedef struct trap_frame_s {
    // registers as pushed by pusha
    uint32 edi;
    uint32 esi;
    uint32 ebp;
    uint32 oesp;      // useless & ignored
    uint32 ebx;
    uint32 edx;
    uint32 ecx;
    uint32 eax;

    // rest of trap frame
    uint16 gs;
    uint16 padding1;
    uint16 fs;
    uint16 padding2;
    uint16 es;
    uint16 padding3;
    uint16 ds;
    uint16 padding4;
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
    void do_isr(trap_frame_t* frame);

private:
	void init_gdt();
	void init_idt();
	void init_traps();
	void init_isrs();

	void set_gate(uint32 index, uint32 addr, uint64 flag);
	void set_trap_gate(uint32 index, uint32 addr);
	void set_interrupt_gate(uint32 index, uint32 addr);

private:
	uint64	m_gdt[GDT_LEN];
	uint64	m_idt[IDT_LEN];
	uint32	m_isr_table[NR_IRQ];
};

class I8259a {
public:
	I8259a();
	~I8259a();

	void init();

private:

private:
};

class Arch {
public:
	Arch();
	~Arch();

	void init();

    CPU* get_cpu();

private:
	CPU		m_cpu;
	I8259a	m_8259a;
};

#endif
