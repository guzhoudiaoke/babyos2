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
#define SEG_TSS				(5)
#define GDT_LEN			    (6)
#define IDT_LEN				(256)

#define	SECT_SIZE			(512)
#define KSTACK_SIZE         (8192)
#define STACK_BOOT			(0x1000)
#define STACK_PM_BOTTOM     (0x10000)


/* boot information */
#define BOOT_INFO_ADDR		(0x8000)    /* 0x8000, 32k */
#define BOOT_INFO_SEG		(BOOT_INFO_ADDR >> 4)

/* 1. video info */
#define VIDEO_INFO_ADDR	    (BOOT_INFO_ADDR)
#define VIDEO_INFO_OFFSET	(VIDEO_INFO_ADDR - BOOT_INFO_ADDR)
#define VIDEO_INFO_SIZE	    (12)

/* 2. memory info */
#define MEMORY_INFO_ADDR	(VIDEO_INFO_ADDR + VIDEO_INFO_SIZE)
#define MEMORY_INFO_OFFSET  (MEMORY_INFO_ADDR - BOOT_INFO_ADDR)
#define MEMORY_INFO_SIZE	(4+256)

/* ap start args */
#define AP_PGDIR            (MEMORY_INFO_ADDR + MEMORY_INFO_SIZE)
#define AP_KSTACK           (AP_PGDIR + 4)
#define AP_MAIN             (AP_KSTACK + 4)
#define AP_INDEX            (AP_MAIN + 4)

#define AP_START_ADDR       (0x9000)    /* 36k, APs will start from here */

/* font */
#define FONT_ASC16_ADDR     (0x10000)   /* 64k */


/* struct of boot disk */
#define LOADER_SECT_NUM     (2)
#define ELF_SECT_NUM        (1126)
#define FONT_ASC16_SECT_NUM (8)

#define BOOT_LBA            (0)
#define LOADER_LBA          (1)
#define KERNEL_ELF_LBA      (3)
#define FONT_ASC16_LBA      (2040)

/* memory */
#define KERNEL_BASE		    0xc0000000  // 3GB
#define EXTENED_MEM         0x100000    // 1MB
#define KERNEL_LOAD         (KERNEL_BASE+EXTENED_MEM)
#define MAX_PHY_MEM         0x10000000  // 256MB
#define USER_STACK_TOP      0xc0000000

/* page table, page directory entry flag */
#define PTE_P               0x001		// present
#define PTE_W               0x002		// writeable
#define PTE_U               0x004		// user

/* for cr0 */
#define CR0_PE              0x00000001
#define CR0_PG              0x80000000
#define CR0_WP              0x00010000

#endif

