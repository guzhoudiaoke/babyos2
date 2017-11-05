/*
 * guzhoudiaoke@126.com
 * 2017-10-23
 */

#include "babyos.h"
#include "kernel.h"
#include "mm.h"
#include "x86.h"
#include "console.h"
#include "string.h"

__attribute__ ((__aligned__(PAGE_SIZE)))
    uint8 kernel_stack[KSTACK_SIZE] = {
        0xff,
    };

/* pg_dir and pte for entry */
__attribute__ ((__aligned__(PAGE_SIZE)))
    pte_t entry_pg_table0[NR_PTE_PER_PAGE] = { 
        [0] = (0) | PTE_P | PTE_W,
    };
__attribute__ ((__aligned__(PAGE_SIZE)))
    pte_t entry_pg_table_vram[NR_PTE_PER_PAGE] = {
        [0] = (0) | PTE_P | PTE_W,
    };

__attribute__ ((__aligned__(PAGE_SIZE)))
    pde_t entry_pg_dir[1024] = { 
        [0] = (0) | PTE_P | PTE_W,
    };

extern uint8 data[];    // defined by kernel.ld
extern uint8 end[];     // defined by kernel.ld

mm_t::mm_t()
{
}
mm_t::~mm_t()
{
}

void *mm_t::boot_mem_alloc(uint32 size, uint32 page_align)
{
    if (page_align) {
        m_mem_start = (uint8 *)PAGE_ALIGN(m_mem_start);
    }

    void *p = (void *) m_mem_start;
    m_mem_start += size;

    return p;
}

void mm_t::map_pages(pde_t *pg_dir, void *va, uint32 pa, uint32 size, uint32 perm)
{
    uint8 *v = (uint8 *) (((uint32)va) & PAGE_MASK);
    uint8 *e = (uint8 *) (((uint32)va + size) & PAGE_MASK);
    pa = (pa & PAGE_MASK);

    pde_t *pde = &pg_dir[PD_INDEX(va)];
    pte_t *pg_table;
    while (v < e) {
        if ((*pde) & PTE_P) {
            pg_table = (pte_t *)(PA2VA((*pde) & PAGE_MASK));
        }
        else {
            pg_table = (pte_t *)boot_mem_alloc(PAGE_SIZE, 1);
            memset(pg_table, 0, PAGE_SIZE);
            *pde = (VA2PA(pg_table) | PTE_P | PTE_W | 0x04);
        }

        pde++;
        for (uint32 i = PT_INDEX(v); i < NR_PTE_PER_PAGE && v < e; i++, v += PAGE_SIZE, pa += PAGE_SIZE) {
            pte_t *pte = &pg_table[i];
            if (v < e) {
                *pte = pa | PTE_P | perm;
            }
        }
    }
}

void mm_t::test_page_mapping()
{
    uint32 total = 0;
    for (uint8 *v = (uint8 *)KERNEL_BASE; v < m_mem_end; v += 1*KB) {
        pde_t *pde = &m_kernel_pg_dir[PD_INDEX(v)];

        if ((*pde) & PTE_P) {
            pte_t *pg_table = (pte_t *)(PA2VA(((*pde) & PAGE_MASK)));
            pte_t *pte = &pg_table[PT_INDEX(v)];
            if (!((*pte) & PTE_P)) {
                console()->kprintf(WHITE, "page fault: v: 0x%p, *pde: 0x%p, *pte: 0x%p\n", v, *pde, *pte);
                break;
            }
        }
        else {
            console()->kprintf(WHITE, "page fault2: v: 0x%p, *pde: 0x%p\n", v, *pde);
            break;
        }

        uint8 x = *v;
        total += x;
    }
}

void mm_t::init_paging()
{
    // mem for m_kernel_pg_dir
    m_kernel_pg_dir = (pde_t *)boot_mem_alloc(PAGE_SIZE, 1);
    memset(m_kernel_pg_dir, 0, PAGE_SIZE);

    // first 1MB: KERNEL_BASE ~ KERNEL_LOAD -> 0~1M
    map_pages(m_kernel_pg_dir, (uint8 *)KERNEL_BASE, 0, EXTENED_MEM, PTE_W);

    // kernel text + rodata: KERNEL_LOAD ~ data -> 1M ~ VA2PA(data)
    map_pages(m_kernel_pg_dir, (uint8 *)KERNEL_LOAD, VA2PA(KERNEL_LOAD), VA2PA(data) - VA2PA(KERNEL_LOAD), 0);

    // kernel data + memory: data ~ KERNEL_BASE+MAX_PHY_MEM -> VA2PA(data) ~ MAX_PHY_MEM
    map_pages(m_kernel_pg_dir, data, VA2PA(data), VA2PA(m_mem_end) - VA2PA(data), PTE_W);

    // map the video vram mem
    uint32 screen_vram = (uint32)os()->get_screen()->vram();
    m_kernel_pg_dir[((uint32)screen_vram)>>22] = ((uint32)(VA2PA(entry_pg_table_vram)) | (PTE_P | PTE_W));

    set_cr3(VA2PA(m_kernel_pg_dir));

    // FIXME: debug
    test_page_mapping();
}

void mm_t::init_mem_range()
{
    memory_layout_t *info = (memory_layout_t *) PA2VA(MEMORY_INFO_ADDR);
    console()->kprintf(WHITE, "the memory info from int 0x15, eax=0xe820:\n");
    console()->kprintf(WHITE, "type\t\taddress\t\tlength\n");
    for (uint32 i = 0; i < info->num_of_range; i++) {
        console()->kprintf(RED, "%x\t%x\t%x\n", 
                info->ranges[i].type,
                info->ranges[i].base_addr_low,
                info->ranges[i].base_addr_low + info->ranges[i].length_low);

        /* I am not sure how to get the total memory, and I don't know why when I set the machine with 128MB
         * memory, this method get a range 1MB ~ 127MB with type 1 (Usable (normal) RAM, 
         * and 127MB~128MB with type 2(Reserved - unusable)
         * for now, get the range of address >= 1M and type == 1 (Usable (normal) RAM */
        if (info->ranges[i].type == 1 && info->ranges[i].base_addr_low >= 1*MB) {
            m_usable_phy_mem_start = info->ranges[i].base_addr_low;
            m_usable_phy_mem_end = info->ranges[i].base_addr_low + info->ranges[i].length_low;
        }
    }

    console()->kprintf(WHITE, "usable memory above 1MB: from %uMB, to %dMB\n", 
            m_usable_phy_mem_start / (1*MB), m_usable_phy_mem_end / (1*MB));

    m_mem_start = end;
    m_mem_end = (uint8 *)PA2VA(m_usable_phy_mem_end);
    console()->kprintf(WHITE, "mem_start: 0x%x, mem_end: 0x%x\n", m_mem_start, m_mem_end);
}

void mm_t::init_free_area()
{
}

void mm_t::init()
{
    init_mem_range();
    init_paging();
    init_free_area();
}

