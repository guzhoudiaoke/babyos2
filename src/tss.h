/*
 * guzhoudiaoke@126.com
 * 2018-02-07
 * separate from cpu.h
 */

#ifndef _TSS_H_
#define _TSS_H_

#define IO_BITMAP_SIZE		(32)
#define INVALID_IO_BITMAP	(0x8000)

/* tss struct defined in linux */
typedef struct tss_s {
    uint16	back_link,__blh;
    uint32	esp0;
    uint16	ss0,__ss0h;
    uint32	esp1;
    uint16	ss1,__ss1h;
    uint32	esp2;
    uint16	ss2,__ss2h;
    uint32  __cr3;
    uint32  eip;
    uint32  eflags;
    uint32  eax,ecx,edx,ebx;
    uint32  esp;
    uint32  ebp;
    uint32  esi;
    uint32  edi;
    uint16	es, __esh;
    uint16	cs, __csh;
    uint16	ss, __ssh;
    uint16	ds, __dsh;
    uint16	fs, __fsh;
    uint16	gs, __gsh;
    uint16	ldt, __ldth;
    uint16	trace, bitmap;
    uint32	io_bitmap[IO_BITMAP_SIZE+1];
    /*
     * pads the TSS to be cacheline-aligned (size is 0x100)
     */
    uint32 __cacheline_filler[5];
} tss_t;


#endif
