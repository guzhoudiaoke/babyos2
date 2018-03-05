/*
 * guzhoudiaoke@126.com
 * 2017-10-21
 */

#ifndef _X86_H_
#define _X86_H_

static inline uint8 inb(uint16 port)
{
    uint16 data;
    __asm__ volatile("inb %1,%0" : "=a" (data) : "d" (port));
    return data;
}

static inline uint8 inw(uint16 port)
{
    uint16 data;
    __asm__ volatile("inw %1,%0" : "=a" (data) : "d" (port));
    return data;
}

static inline void insl(int port, void *addr, int cnt)
{
    __asm__ volatile("cld; rep insl" :
            "=D" (addr), "=c" (cnt) :
            "d" (port), "0" (addr), "1" (cnt) :
            "memory", "cc");
}

static inline void outb(uint16 port, uint8 data)
{
    __asm__ volatile("outb %0,%1" : : "a" (data), "d" (port));
}

static inline void outw(uint16 port, uint16 data)
{
    __asm__ volatile("outw %0,%1" : : "a" (data), "d" (port));
}

static inline void outl(uint16 port, uint32 val)
{
	asm volatile("outl %0,%1" : : "a" (val), "dN" (port));
}

static inline uint32 inl(uint16 port)
{
	uint32 v;
	asm volatile("inl %1,%0" : "=a" (v) : "dN" (port));
	return v;
}

static inline void outsl(int port, const void *addr, int cnt)
{
    __asm__ volatile("cld; rep outsl" :
            "=S" (addr), "=c" (cnt) :
            "d" (port), "0" (addr), "1" (cnt) :
            "cc");
}

static inline void stosb(void *addr, int32 data, int32 cnt)
{
    __asm__ volatile("cld; rep stosb" :
            "=D" (addr), "=c" (cnt) :
            "0" (addr), "1" (cnt), "a" (data) :    /* di=addr, cx=cnt, ax=data */
            "memory", "cc");
}

static inline void movsb(void *dst, void *src, int32 cnt)
{
    __asm__ volatile("cld; rep movsb" :
            :
            "D" (dst), "S" (src), "c" (cnt) :      /* di=dst, si=src, cx=cnt */
            "memory", "cc");
}

static inline uint32 get_cr0(void)
{
    uint32 val;
    __asm__ volatile("movl %%cr0, %0" : "=r" (val));
    return val;
}

static inline void set_cr0(uint32 val)
{
    __asm__ volatile("movl %0, %%cr0" : : "r" (val));
}

static inline void set_cr3(uint32 val)
{
    __asm__ volatile("movl %0, %%cr3" : : "r" (val));
}

#define CMOS_ADDR_PORT		0x70
#define CMOS_DATA_PORT		0x71

static inline uint32 cmos_read(uint32 reg)
{
    outb(CMOS_ADDR_PORT, reg);
    return inb(CMOS_DATA_PORT);
}

static inline void cmos_write(uint32 reg, uint32 value)
{
    outb(CMOS_ADDR_PORT, reg);
    outb(CMOS_DATA_PORT, value);
}

static inline uint32 nvram_read(uint32 reg)
{
    return cmos_read(reg) | cmos_read(reg+1) << 8;
}

static inline void lgdt(void* gdt, uint32 size)
{
    volatile uint16 pd[3];

    pd[0] = size-1;
    pd[1] = (uint32)gdt;
    pd[2] = (uint32)gdt >> 16;

    __asm__ volatile("lgdt (%0)" : : "r" (pd));
}

static inline void lidt(void* idt, uint32 size)
{
    volatile uint16 pd[3];

    pd[0] = size-1;
    pd[1] = (uint32)idt;
    pd[2] = (uint32)idt >> 16;

    __asm__ volatile("lidt (%0)" : : "r" (pd));
}

static inline void ltr(uint16 sel)
{
    __asm__ volatile("ltr %0" : : "r" (sel));
}

static inline void sti(void)
{
    __asm__ volatile("sti");
}

static inline void cli(void)
{
    __asm__ volatile("cli");
}

static inline void halt(void)
{
    __asm__ volatile("hlt");
}

static inline uint32 xchg(volatile uint32 *addr, uint32 newval)
{
    uint32 result;

    // "+m": read-modify-write operand
    __asm__ volatile("lock; xchgl %0, %1" :
            "+m" (*addr), "=a" (result) :
            "1" (newval) :
            "cc");
    return result;
}

static inline int change_bit(int nr, void* addr)
{
    int oldbit;

    __asm__ __volatile__("btcl %2,%1\n\tsbbl %0,%0"
            :"=r" (oldbit),"=m" (*((unsigned *)(addr)))
            :"r" (nr));
    return oldbit;
}

#define rdtsc(low,high) \
     __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))

#define rdtsc64(val) \
     __asm__ __volatile__("rdtsc" : "=A" (val))

#define rdmsr(msr,val1,val2) \
	__asm__ __volatile__("rdmsr" \
			  : "=a" (val1), "=d" (val2) \
			  : "c" (msr))

#define wrmsr(msr,val1,val2) \
	__asm__ __volatile__("wrmsr" \
			  : /* no outputs */ \
			  : "c" (msr), "a" (val1), "d" (val2))


#endif

static inline void nop(void)
{
    __asm__ __volatile__("rep; nop");
}

#define mb() 	__asm__ __volatile__ ("lock; addl $0,0(%%esp)": : :"memory")
#define rmb()	mb()
#define wmb()	__asm__ __volatile__ ("": : :"memory")

#define barrier() __asm__ __volatile__("": : :"memory")

