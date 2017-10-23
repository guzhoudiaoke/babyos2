/*
 *
 *
 */

#ifndef _MM_H_
#define _MM_H_

#include "types.h"

// page table, page directory entry flag
#define PTE_P           0x001   // present
#define PTE_W           0x002   // writeable


#define PAGESIZE        4096
#define KERNEL_BASE		0xc0000000

#define VA_2_PA(x)	(((uint32)(x)) - KERNEL_BASE)
#define PA_2_VA(x)	(((void *)(x)) + KERNEL_BASE)

typedef uint32  pde_t;
typedef uint32  pte_t;

void init_mm();
void kmap_device(void *va);

#endif
