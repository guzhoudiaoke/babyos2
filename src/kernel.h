/*
 * guzhoudiaoke@126.com
 * 2017-10-21
 */

#ifndef _KERNEL_H_
#define _KENERL_H_

/* segs */
#define SEG_NULL			(0)
#define SEG_KCODE           (1)
#define SEG_KDATA           (2)
#define SEG_UCODE           (3)
#define SEG_UDATA           (4)

#define GDT_LEN			    (5)
#define IDT_LEN				(256)


#define	SECT_SIZE			(512)
#define LOADER_SECT_NUM     (1)
#define KERNEL_SECT_NUM	    (256)

/* address of temp kernel */
#define TMP_KERNEL_ADDR	    (0x10000)

#define KSTACK_SIZE         (4096)
#define STACK_BOOT			(0x1000)
#define STACK_PM_BOTTOM     (0x10000)


/* boot information */
#define BOOT_INFO_ADDR		(0x90000)
#define BOOT_INFO_SEG		(BOOT_INFO_ADDR >> 4)

#define GDT_ADDR			(BOOT_INFO_ADDR)
#define GDT_SIZE			(8 * GDT_LEN)

#define VIDEO_INFO_ADDR	    (GDT_ADDR + GDT_SIZE)
#define VIDEO_INFO_OFFSET	(VIDEO_INFO_ADDR - BOOT_INFO_ADDR)
#define VIDEO_INFO_SIZE	    (12)

#define MEMORY_INFO_ADDR	(VIDEO_INFO_ADDR + VIDEO_INFO_SIZE)
#define MEMORY_INFO_OFFSET  (MEMORY_INFO_ADDR - BOOT_INFO_ADDR)
#define MEMORY_INFO_SIZE	(4+256)


/* load addresses */
#define LOADER_ADDR         (TMP_KERNEL_ADDR)

#define ELF_BASE_ADDR       (LOADER_ADDR + LOADER_SECT_NUM * SECT_SIZE)
#define ELF_SECT_NUM        (242)

#define FONT_ASC16_ADDR     (ELF_BASE_ADDR + ELF_SECT_NUM * SECT_SIZE)
#define FONT_ASC16_SIZE     (4096)


/* memory */
#define KERNEL_BASE		    0xc0000000  // 3GB
#define EXTENED_MEM         0x100000    // 1MB
#define KERNEL_LOAD         (KERNEL_BASE+EXTENED_MEM)
#define MAX_PHY_MEM         0x10000000  // 256MB


/* page table, page directory entry flag */
#define PTE_P               0x001		// present
#define PTE_W               0x002		// writeable


/* for cr0 */
#define CR0_PE              0x00000001
#define CR0_WP              0x00010000
#define CR0_PG              0x80000000

#endif

