/*
 * guzhoudiaoke@126.com
 * 2018-01-28
 */

#ifndef _TRAPS_H_
#define _TRAPS_H_

#define IRQ_0               (0x20)
#define IRQ_NUM             (0x10)

#define IRQ_TIMER           (0x00)
#define IRQ_KEYBOARD        (0x01)
#define IRQ_HARDDISK        (0x0e)

#define VEC_LOCAL_TIMER     (0xfc)
#define VEC_THERMAL         (0xfd)
#define VEC_ERROR           (0xfe)
#define VEC_SPURIOUS        (0xff)

#define IRQ_SYSCALL         (0x80)
#define INT_PF				(14)

typedef struct trap_frame_s {
    uint32 ebx;
    uint32 ecx;
    uint32 edx;
    uint32 esi;
    uint32 edi;
    uint32 ebp;
    uint32 eax;

    uint16 gs, padding4;
    uint16 fs, padding3;
    uint16 ds, padding1;
    uint16 es, padding2;

    uint32 trapno;

    // pushed by x86 hardware
    uint32 err;
    uint32 eip;
    uint16 cs;
    uint16 padding5;
    uint32 eflags;

    // cross rings
    uint32 esp;
    uint16 ss, padding6;
} trap_frame_t;


#endif
