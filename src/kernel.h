/*
 * guzhoudiaoke@126.com
 * 2017-10-21
 */

#ifndef _KERNEL_H_
#define _KENERL_H_

#define SEG_KCODE   1
#define SEG_KDATA   2
#define SEG_UCODE   3
#define SEG_UDATA   4
#define SEG_TSS     5


#define SECT_SIZE           512

/* address of temp kernel */
#define TMP_KERNEL_ADDR     0x10000

/* address gdt */
#define	GDT_ADDR			(0x90000)
#define	GDT_LEN				(5)
#define	GDT_SIZE			(8 * GDT_LEN)

/* address of video information */
#define	VIDEO_INFO_ADDR		(GDT_ADDR + GDT_SIZE)


#endif

