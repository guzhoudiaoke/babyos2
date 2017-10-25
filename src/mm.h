/*
 * guzhoudiaoke@126.com
 * 2017-10-23
 */

#ifndef _MM_H_
#define _MM_H_

#include "types.h"

typedef uint32  pde_t;
typedef uint32  pte_t;

#define NR_PDE_PER_PAGE     (PAGE_SIZE / sizeof(pde_t))
#define NR_PTE_PER_PAGE     (PAGE_SIZE / sizeof(pte_t))

typedef struct address_range_s {
	uint32	base_addr_low;
	uint32	base_addr_high;
	uint32	length_low;
	uint32	lenght_high;
    uint32  type;
} address_range_t;
typedef struct boot_memory_info_s {
	uint32 num_of_range;
	address_range_t ranges[32];
} boot_memory_info_t;

void init_mm();
void kmap_device(void *va);

#endif

