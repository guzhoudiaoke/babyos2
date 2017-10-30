/*
 * guzhoudiaoke@126.com
 * 2017-10-23
 */

#ifndef _mm_t_H_
#define _mm_t_H_

#include "types.h"

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

#define VA2PA(x)	        (((uint32)(x)) - KERNEL_BASE)
#define PA2VA(x)	        ((void *)((x) + KERNEL_BASE))
#define NR_PDE_PER_PAGE     (PAGE_SIZE / sizeof(pde_t))
#define NR_PTE_PER_PAGE     (PAGE_SIZE / sizeof(pte_t))

class BabyOS;
class mm_t {
public:
	mm_t();
	~mm_t();

	void init();
	void *boot_mem_alloc(uint32 size, uint32 page_align);

private:
	void map_pages(pde_t *pg_dir, void *va, uint32 pa, uint32 size, uint32 perm);
	void test_page_mapping();
	void init_paging();
	void init_mem_range();
	void init_free_area();

private:
	pde_t *m_kernel_pg_dir;

	uint8 *m_mem_start;
	uint8 *m_mem_end;

	uint32 m_usable_phy_mem_start;
	uint32 m_usable_phy_mem_end;
};

#endif

