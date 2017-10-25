/*
 * guzhoudiaoke@126.com
 * 2017-10-21
 */

#ifndef _X86_H_
#define _X86_H_

static inline uint8 inb(uint16 port)
{
    uint8 data;

    __asm__ volatile("in %1,%0" : "=a" (data) : "d" (port));
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
    __asm__ volatile("out %0,%1" : : "a" (data), "d" (port));
}

static inline void outw(uint16 port, uint16 data)
{
    __asm__ volatile("out %0,%1" : : "a" (data), "d" (port));
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

#define NVRAM_BASELOW		(0x15)
#define NVRAM_BASEHIGH		(0x16)
#define NVRAM_EXTLOW		(0x17)
#define NVRAM_EXTHIGH		(0x18)

static inline uint32 cmos_read(uint32 reg)
{
	outb(CMOS_ADDR_PORT, reg);
	return inb(CMOS_DATA_PORT);
}

static inline uint32 nvram_read(uint32 reg)
{
	return cmos_read(reg) | cmos_read(reg+1) << 8;
}

#endif

