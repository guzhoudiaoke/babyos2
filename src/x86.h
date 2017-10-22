/*
 * guzhoudiaoke@126.com
 * 2017-10-21
 */

#ifndef _X86_H_
#define _X86_H_

static inline uint8 inb(uint16 port)
{
    uint8 data;

    asm volatile("in %1,%0" : "=a" (data) : "d" (port));
    return data;
}

static inline void insl(int port, void *addr, int cnt)
{
    asm volatile("cld; rep insl" :
                 "=D" (addr), "=c" (cnt) :
                 "d" (port), "0" (addr), "1" (cnt) :
                 "memory", "cc");
}

static inline void outb(uint16 port, uint8 data)
{
    asm volatile("out %0,%1" : : "a" (data), "d" (port));
}

static inline void outw(uint16 port, uint16 data)
{
    asm volatile("out %0,%1" : : "a" (data), "d" (port));
}

static inline void outsl(int port, const void *addr, int cnt)
{
    asm volatile("cld; rep outsl" :
                 "=S" (addr), "=c" (cnt) :
                 "d" (port), "0" (addr), "1" (cnt) :
                 "cc");
}

static inline void stosb(void *addr, int32 data, int32 cnt)
{
    asm volatile("cld; rep stosb" :
                 "=D" (addr), "=c" (cnt) :
                 "0" (addr), "1" (cnt), "a" (data) :
                 "memory", "cc");
}

#endif

