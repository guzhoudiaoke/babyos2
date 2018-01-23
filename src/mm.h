/*
 * guzhoudiaoke@126.com
 * 2017-10-23
 */

#ifndef _mm_t_H_
#define _mm_t_H_

#include "types.h"
#include "atomic.h"

#define KB                  (1024)
#define MB                  (1024*KB)
#define GB                  (1024*GB)

#define PAGE_SHIFT          (12)
#define PAGE_SIZE           (1U << PAGE_SHIFT)
#define PAGE_MASK           (~(PAGE_SIZE-1))
#define PAGE_ALIGN(addr)    (((uint32)(addr)+PAGE_SIZE-1) & PAGE_MASK)

#define PGDIR_SHIFT	        (22)
#define PGDIR_SIZE	        (1UL << PGDIR_SHIFT)

#define PD_INDEX(va)        (((uint32)va >> 22) & 0x3ff)
#define PT_INDEX(va)        (((uint32)va >> 12) & 0x3ff)

#define PTE_ADDR(pte)       ((uint32)(pte) & ~0xfff)
#define PTE_FLAG(pte)       ((uint32)(pte) & 0xfff)

#define VA2PA(x)	        (((uint32)(x)) - KERNEL_BASE)
#define PA2VA(x)	        ((void *)((x) + KERNEL_BASE))
#define NR_PDE_PER_PAGE     (PAGE_SIZE / sizeof(pde_t))
#define NR_PTE_PER_PAGE     (PAGE_SIZE / sizeof(pte_t))

#define MAX_ORDER           6
#define MAP_NR(addr)		(((unsigned long)(addr)) >> PAGE_SHIFT)


typedef struct page_s {
	atomic_t	ref;
} page_t;

typedef struct free_list_s {
	struct free_list_s*	next;
	struct free_list_s*	prev;
	uint32*				map;
} free_list_t;

typedef struct free_area_s {
	free_list_t free_list[MAX_ORDER+1];
	uint8*		base;
} free_area_t;


class mm_t {
public:
	mm_t();
	~mm_t();

	void init();
	void* boot_mem_alloc(uint32 size, uint32 page_align);
    pde_t* get_kernel_pg_dir() { return m_kernel_pg_dir; } 
	void map_pages(pde_t *pg_dir, void *va, uint32 pa, uint32 size, uint32 perm);

    void* alloc_pages(uint32 order);
    void  free_pages(void* addr, uint32 order);

    void inc_page_ref(uint32 phy_addr);
    uint32 dec_page_ref(uint32 phy_addr);
    uint32 get_page_ref(uint32 phy_addr);
    uint32 va_2_pa(void* va);
    void   copy_page(void* dst, void* src);

private:
    void test_page_mapping();
    void init_paging();
    void init_mem_range();
    void init_free_area();
    void boot_map_pages(pde_t *pg_dir, void *va, uint32 pa, uint32 size, uint32 perm);

    uint32 get_buddy(uint32 addr, uint32 mask);
    int mark_used(uint32 addr, uint32 order);
    void* expand(free_list_t* addr, uint32 low, uint32 high);
    void free_boot_mem();

    void init_pages();

private:
    pde_t *m_kernel_pg_dir;
    page_t *m_pages;

    uint8 *m_mem_start;
    uint8 *m_mem_end;

    uint32 m_usable_phy_mem_start;
    uint32 m_usable_phy_mem_end;

    free_area_t m_free_area;
    atomic_t m_free_page_num;
};

#endif

