/*
 * guzhoudiaoke@126.com
 * 2017-10-21
 */

#ifndef _KERNEL_H_
#define _KENERL_H_

/* segs */
#define SEG_KCODE           (1)
#define SEG_KDATA           (2)
#define SEG_UCODE           (3)
#define SEG_UDATA           (4)
#define SEG_TSS             (5)


#define	SECT_SIZE			(512)
#define LOADER_SECT_NUM     (1)
#define KERNEL_SECT_NUM	    (256)

/* address of temp kernel */
#define TMP_KERNEL_ADDR	    (0x10000)

#define KSTACK_SIZE         (4096)
#define STACK_BOOT			(0x1000)
#define STACK_PM_BOTTOM     (0x10000)


/* address gdt */
#define GDT_ADDR			(0x90000)
#define GDT_LEN			    (5)
#define GDT_SIZE			(8 * GDT_LEN)

#define VIDEO_INFO_ADDR	    (GDT_ADDR + GDT_SIZE)
#define VIDEO_INFO_SIZE	    (12)

#define MEMORY_INFO_ADDR	(VIDEO_INFO_ADDR + VIDEO_INFO_SIZE)
#define MEMORY_INFO_SIZE	(4+256)
#define MEMORY_INFO_SEG     (MEMORY_INFO_ADDR >> 4)
#define MEMORY_INFO_OFFSET  (MEMORY_INFO_ADDR - (MEMORY_INFO_SEG << 4))

#define LOADER_ADDR         (TMP_KERNEL_ADDR)

#define ELF_BASE_ADDR       (LOADER_ADDR + LOADER_SECT_NUM * SECT_SIZE)
#define ELF_SECT_NUM        (128)

#define FONT_ASC16_ADDR     (ELF_BASE_ADDR + ELF_SECT_NUM * SECT_SIZE)
#define FONT_ASC16_SIZE     (4096)



// page table, page directory entry flag
#define PTE_P               0x001   // present
#define PTE_W               0x002   // writeable

#define KB                  1024
#define MB                  (1024*KB)
#define GB                  (1024*GB)

#define PAGE_SHIFT          12
#define PAGE_SIZE           (1UL << PAGE_SHIFT)
#define PAGE_MASK           (~(PAGE_SIZE-1))
#define PAGE_ALIGN(addr)    (((uint32)(addr)+PAGE_SIZE-1) & PAGE_MASK)

#define PD_INDEX(va)        (((uint32)va >> 22) & 0x3ff)
#define PT_INDEX(va)        (((uint32)va >> 12) & 0x3ff)

#define PTE_ADDR(pte)       ((uint32)(pte) & ~0xfff)
#define PTE_FLAG(pte)       ((uint32)(pte) & 0xfff)

#define KERNEL_BASE		    0xc0000000  // 3GB
#define EXTENED_MEM         0x100000    // 1MB
#define KERNEL_LOAD         (KERNEL_BASE+EXTENED_MEM)
#define MAX_PHY_MEM         0x10000000  // 256MB

#define VA2PA(x)	        (((uint32)(x)) - KERNEL_BASE)
#define PA2VA(x)	        (((void *)(x)) + KERNEL_BASE)


// for cr0
#define CR0_PE              0x00000001
#define CR0_WP              0x00010000
#define CR0_PG              0x80000000

#endif

