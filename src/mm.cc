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
#include "process.h"
#include "math.h"
#include "local_apic.h"

__attribute__ ((__aligned__(KSTACK_SIZE)))
uint8 kernel_stack[8*KSTACK_SIZE] = {
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
pde_t entry_pg_dir[NR_PDE_PER_PAGE] = { 
    [0] = (0) | PTE_P | PTE_W,
};

extern uint8 data[];    // defined by kernel.ld
extern uint8 end[];     // defined by kernel.ld


static inline void add_to_head(free_list_t* head, free_list_t * entry)
{
    entry->prev = head;
    entry->next = head->next;
    head->next->prev = entry;
    head->next = entry;
}

static inline void remove_head(free_list_t* head, free_list_t * entry)
{
    entry->next->prev = entry->prev;
    entry->prev->next = entry->next;
}

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

void mm_t::boot_map_pages(pde_t *pg_dir, void *va, uint32 pa, uint32 size, uint32 perm)
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

void mm_t::map_pages(pde_t *pg_dir, void *va, uint32 pa, uint32 size, uint32 perm)
{
    //console()->kprintf(YELLOW, "map va: %x to pa: %p\n", va, pa);

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
            pg_table = (pte_t *)alloc_pages(0);
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
    asm volatile("movl %0, %%cr3": :"r" (VA2PA(pg_dir)));
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
    m_kernel_pg_dir = (pde_t *) boot_mem_alloc(PAGE_SIZE, 1);
    memset(m_kernel_pg_dir, 0, PAGE_SIZE);

    // first 1MB: KERNEL_BASE ~ KERNEL_LOAD -> 0~1M
    boot_map_pages(m_kernel_pg_dir, (uint8 *)KERNEL_BASE, 0, EXTENED_MEM, PTE_W);

    // kernel text + rodata: KERNEL_LOAD ~ data -> 1M ~ VA2PA(data)
    boot_map_pages(m_kernel_pg_dir, (uint8 *)KERNEL_LOAD, VA2PA(KERNEL_LOAD), VA2PA(data) - VA2PA(KERNEL_LOAD), 0);

    // kernel data + memory: data ~ KERNEL_BASE+MAX_PHY_MEM -> VA2PA(data) ~ MAX_PHY_MEM
    boot_map_pages(m_kernel_pg_dir, data, VA2PA(data), VA2PA(m_mem_end) - VA2PA(data), PTE_W);

    // map the video vram mem
    uint32 screen_vram = (uint32)os()->get_screen()->vram();
    m_kernel_pg_dir[((uint32)screen_vram)>>22] = ((uint32)(VA2PA(entry_pg_table_vram)) | (PTE_P | PTE_W));

    // map apic base
    pte_t* pg_table_apic = (pte_t *) boot_mem_alloc(PAGE_SIZE, 1);
    pg_table_apic[PT_INDEX(APIC_BASE)] = APIC_BASE | (PTE_P | PTE_W);
    pg_table_apic[PT_INDEX(IO_APIC_BASE)] = IO_APIC_BASE | (PTE_P | PTE_W);
    m_kernel_pg_dir[PD_INDEX(APIC_BASE)] = ((uint32)(VA2PA(pg_table_apic)) | (PTE_P | PTE_W));

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
        console()->kprintf(RED, "0x%8x\t0x%8x\t0x%8x\n", 
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
    console()->kprintf(WHITE, "mem_start: 0x%8x, mem_end: 0x%8x\n", m_mem_start, m_mem_end);
}

void mm_t::init_pages()
{
    uint32 page_num = (uint32) (m_mem_end - KERNEL_BASE) / PAGE_SIZE;
    uint32 size = page_num * sizeof(page_t);
    m_pages = (page_t *) boot_mem_alloc(size, 0);

    for (uint32 i = 0; i < page_num; i++) {
        atomic_set(&m_pages[i].ref, 1);
    }
}

void mm_t::init_free_area()
{
    uint32 mask = PAGE_MASK;
    uint32 bitmap_size;
    for (int i = 0; i <= MAX_ORDER; i++) {
        m_free_area.free_list[i].prev = m_free_area.free_list[i].next = &m_free_area.free_list[i];
        mask += mask;
        m_mem_end = (uint8 *)(((uint32)m_mem_end) & mask);
        bitmap_size = (uint32 (m_mem_end - m_mem_start)) >> (PAGE_SHIFT + i);
        bitmap_size = (bitmap_size + 7) >> 3;
        bitmap_size = (bitmap_size + sizeof(uint32) - 1) & ~(sizeof(uint32)-1);
        m_free_area.free_list[i].map = (uint32 *) m_mem_start;
        memset((void *) m_mem_start, 0, bitmap_size);
        m_mem_start += bitmap_size;
    }

    init_pages();

    m_free_area.base = (uint8*)(((uint32)m_mem_start + ~mask) & mask);
    free_boot_mem();
}

void mm_t::init()
{
    init_mem_range();
    init_paging();
    init_free_area();
}

uint32 mm_t::get_buddy(uint32 addr, uint32 mask)
{
    uint32 buddy = ((addr - (uint32)m_free_area.base) ^ (-mask)) + (uint32)m_free_area.base;
    return buddy;
}

int mm_t::mark_used(uint32 addr, uint32 order)
{
    return change_bit(MAP_NR(addr - (uint32)m_free_area.base) >> (1+order), m_free_area.free_list[order].map);
}

void mm_t::free_pages(void* addr, uint32 order)
{
    // dec the ref count, if it's not 0, don't free the pages
    if (!dec_page_ref(VA2PA(addr))) {
        return;
    }

    atomic_add(math_t::pow(2, order), &m_free_page_num);

    uint32 address = (uint32) addr;
    uint32 index = MAP_NR(address - (uint32)m_free_area.base) >> (1 + order);
    uint32 mask = PAGE_MASK << order;

    address &= mask;
    while (order < MAX_ORDER) {
        if (!change_bit(index, m_free_area.free_list[order].map)) {
            break;
        }

        uint32 buddy = get_buddy(address, mask);
        remove_head(m_free_area.free_list+order, (free_list_t *)buddy);
        order++;
        index >>= 1;
        mask <<= 1;
        address &= mask;
    }
    add_to_head(m_free_area.free_list+order, (free_list_t *) address);
}

void* mm_t::expand(free_list_t* addr, uint32 low, uint32 high)
{
    uint32 size = PAGE_SIZE << high;
    while (low < high) {
        high--;
        size >>= 1;
        add_to_head(m_free_area.free_list+high, addr);
        mark_used((uint32) addr, high);
        addr = (free_list_t *) (size + (uint32) addr);
    }

    void* p = addr;
    for (uint32 i = 0; i < math_t::pow(2, low); i++, p += PAGE_SIZE) {
        inc_page_ref(VA2PA(p));
    }
    return addr;
}

void* mm_t::alloc_pages(uint32 order)
{
    atomic_sub(math_t::pow(2, order), &m_free_page_num);
    //console()->kprintf(YELLOW, "alloc order: %u, free page num: %u\n", order, atomic_read(&m_free_page_num));

    free_list_t* queue = m_free_area.free_list + order;
    uint32 new_order = order;
    do {
        free_list_t* next = queue->next;
        if (queue != next) {
            queue->next = next->next;
            next->next->prev = queue;
            mark_used((uint32) next, new_order);
            return expand(next, order, new_order);
        }
        new_order++;
        queue++;
    } while (new_order <= MAX_ORDER);

    return NULL;
}

void mm_t::free_boot_mem()
{
    atomic_set(&m_free_page_num, 0);
    for (uint8 *p = m_free_area.base; p < m_mem_end; p += PAGE_SIZE) {
        free_pages(p, 0);
    }
}

void mm_t::inc_page_ref(uint32 phy_addr)
{
    page_t* page = &m_pages[phy_addr >> PAGE_SHIFT];
    atomic_inc(&page->ref);
}

uint32 mm_t::dec_page_ref(uint32 phy_addr)
{
    page_t* page = &m_pages[phy_addr >> PAGE_SHIFT];
    if (page->ref.counter <= 0) {
        os()->panic("ref count <= 0 when dec ref");
    }
    return atomic_dec_and_test(&page->ref);
}

uint32 mm_t::get_page_ref(uint32 phy_addr)
{
    page_t* page = &m_pages[phy_addr >> PAGE_SHIFT];
    return atomic_read(&page->ref);
}

uint32 mm_t::va_2_pa(void* va)
{
    if ((uint32) va >= KERNEL_BASE) {
        return VA2PA(va);
    }

    pde_t* pg_dir = current->m_vmm.get_pg_dir();

    pde_t *pde = &pg_dir[PD_INDEX(va)];
    if (!(*pde & PTE_P)) {
        return -1;
    }

    pte_t* table = (pte_t *) PA2VA((*pde) & PAGE_MASK);
    if (!table[PT_INDEX(va)] & PTE_P) {
        return -1;
    }

    uint32 offset = (uint32) va - ((uint32) va & PAGE_MASK);
    return (table[PT_INDEX(va)] & PAGE_MASK) + offset;
}

void mm_t::copy_page(void* dst, void* src)
{
    dst = PA2VA(va_2_pa(dst) & PAGE_MASK);
    src = PA2VA(va_2_pa(src) & PAGE_MASK);

    memcpy(dst, src, PAGE_SIZE);
}

