/*
 * 2017-11-27
 * guzhoudiaoke@126.com
 */

#include "babyos.h"
#include "vm.h"
#include "x86.h"
#include "string.h"
#include "signal.h"

void vmm_t::init()
{
    m_mmap = NULL;
    m_pg_dir = NULL;
}

uint32 vmm_t::copy(const vmm_t& vmm)
{
    if (copy_page_table(vmm.m_pg_dir)) {
        return -1;
    }

    if (copy_vma(vmm.m_mmap)) {
        return -1;
    }

    return 0;
}

uint32 vmm_t::copy_page_table(pde_t* pg_dir)
{
    m_pg_dir = (pde_t *) os()->get_mm()->alloc_pages(0);
    if (m_pg_dir == NULL) {
        return -1;
    }

    memcpy(m_pg_dir, pg_dir, PAGE_SIZE);
    for (uint32 i = 0; i < KERNEL_BASE/(4*MB); i++) {
        pde_t *pde = &pg_dir[i];
        if (!(*pde & PTE_P)) {
            continue;
        }

        pte_t* table = (pte_t *) PA2VA((*pde) & PAGE_MASK);
        pte_t* new_table = (pte_t *) os()->get_mm()->alloc_pages(0);
        if (new_table == NULL) {
            return -1;
        }

        /* inc page ref count and set write protected */
        for (uint32 j = 0; j < NR_PTE_PER_PAGE; j++) {
            if (table[j] & PTE_P) {
                os()->get_mm()->inc_page_ref((table[j] & PAGE_MASK));
                table[j] &= (~PTE_W);
            }
        }

        /* copy table and set to pg_dir */
        memcpy(new_table, table, PAGE_SIZE);
        m_pg_dir[i] = (VA2PA(new_table) | (*pde & (~PAGE_MASK)));
    }

    asm volatile("movl %0, %%cr3": :"r" (VA2PA(pg_dir)));

    return 0;
}

uint32 vmm_t::copy_vma(vm_area_t* mmap)
{
    vm_area_t* tail = NULL;
    vm_area_t* p = mmap;
    while (p != NULL) {
        vm_area_t* vma = (vm_area_t *) os()->get_obj_pool(VMA_POOL)->alloc_from_pool();
        if (vma == NULL) {
            return -1;
        }

        *vma = *p;
        vma->m_next = NULL;

        if (tail == NULL) {
            m_mmap = vma;
        }
        else {
            tail->m_next = vma;
        }
        tail = vma;

        p = p->m_next;
    }

    return 0;
}

/* Look up the first VMA which satisfies  addr < vm_end,  NULL if none. */
vm_area_t* vmm_t::find_vma(uint32 addr)
{
    vm_area_t* vma = m_mmap;
    while (vma != NULL) {
        if (addr < vma->m_end) {
            return vma;
        }
        vma = vma->m_next;
    }
    return NULL;
}

vm_area_t* vmm_t::find_vma(uint32 addr, vm_area_t*& prev)
{
    prev = NULL;
    vm_area_t* vma = m_mmap;
    while (vma != NULL) {
        if (addr < vma->m_end) {
            return vma;
        }
        prev = vma;
        vma = vma->m_next;
    }
    return NULL;
}

uint32 vmm_t::insert_vma(vm_area_t* vma)
{
    vm_area_t* prev = NULL;
    vm_area_t* p = m_mmap;
    while (p != NULL) {
        if (p->m_start >= vma->m_end) {
            break;
        }
        prev = p;
        p = p->m_next;
    }

    if (prev != NULL && prev->m_end > vma->m_start) {
        console()->kprintf(RED, "insert_vma: inserting: [%x, %x], overlaped with [%x, %x]\n", 
                vma->m_start, vma->m_end, prev->m_start, prev->m_end);
        return -1;
    }

    vma->m_next = p;
    if (prev != NULL) {
        prev->m_next = vma;
    }
    else {
        m_mmap = vma;
    }

    /* merge prev and vma */
    if (prev != NULL && prev->m_end == vma->m_start) {
        if (prev->m_page_prot == vma->m_page_prot && prev->m_flags == vma->m_flags) {
            prev->m_end = vma->m_end;
            prev->m_next = p;
            os()->get_obj_pool(VMA_POOL)->free_object(vma);
            vma = prev;
        }
    }

    /* merge vma and p */
    if (p != NULL && vma->m_end == p->m_start) {
        if (vma->m_page_prot == p->m_page_prot && vma->m_flags == p->m_flags) {
            vma->m_end = p->m_end;
            vma->m_next = p->m_next;
            os()->get_obj_pool(VMA_POOL)->free_object(p);
        }
    }

    return 0;
}

uint32 vmm_t::get_unmapped_area(uint32 len)
{
    uint32 addr = VM_UNMAPPED_BASE;

    vm_area_t* vma = find_vma(addr);
    while (vma != NULL) {
        if (USER_VM_SIZE - len < addr) {
            return 0;
        }

        if (addr + len <= vma->m_start) {
            return addr;
        }
        addr = vma->m_end;
        vma = vma->m_next;
    }

    return addr;
}

/*
 * for now, the mmap should:
 *	1) if MAP_FIXED, addr should align with PAGE_SIZE
 *	2) [addr, addr+len] not intersect with a vma already exist
 */
uint32 vmm_t::do_mmap(uint32 addr, uint32 len, uint32 prot, uint32 flags)
{
    /* make len align with PAGE_SIZE */
    len = PAGE_ALIGN(len);

    /* len is 0, nothing to do */
    if (len == 0) {
        return addr;
    }

    /* out of range */
    if (len > USER_VM_SIZE || addr > USER_VM_SIZE || addr > USER_VM_SIZE - len) {
        return -1;
    }

    /* if MAP_FIXED, the addr should align with PAGE_SIZE */
    if (flags & MAP_FIXED) {
        if (addr & ~PAGE_MASK) {
            return -1;
        }

        /* check [addr, addr+len] not in a vm_area */
        vm_area_t* p = find_vma(addr);
        if (p != NULL && addr + len > p->m_start) {
            console()->kprintf(RED, "do_mmap: addr: %p is overlaped with vma: [%p, %p]\n", 
                    addr, p->m_start, p->m_end);
            return -1;
        }
    }
    else {
        addr = get_unmapped_area(len);
        if (addr == 0) {
            console()->kprintf(RED, "do_mmap: failed to get_unmaped_area\n");
            return -1;
        }
    }

    /* alloc a vma from pool */
    vm_area_t* vma = (vm_area_t*) os()->get_obj_pool(VMA_POOL)->alloc_from_pool();
    if (vma == NULL) {
        console()->kprintf(RED, "do_mmap: failed to alloc vma\n");
        return -1;
    }

    /* setup vma */
    vma->m_start = addr;
    vma->m_end = addr + len;
    vma->m_flags = (prot & (VM_READ | VM_WRITE | VM_EXEC));
    vma->m_page_prot = prot;
    vma->m_next = NULL;

    /* insert vma into list, and do merge */
    if (insert_vma(vma)) {
        console()->kprintf(RED, "do_mmap: failed to insert vma: [%x, %x]\n", vma->m_start, vma->m_end);
        return -1;
    }

    return addr;
}

uint32 vmm_t::remove_vma(vm_area_t* vma, vm_area_t* prev)
{
    if (prev != NULL) {
        prev->m_next = vma->m_next;
    }
    else {
        m_mmap = vma->m_next;
    }
    os()->get_obj_pool(VMA_POOL)->free_object(vma);

    return 0;
}

uint32 vmm_t::do_munmap(uint32 addr, uint32 len)
{
    /* addr should align with PAGE_SIZE */
    if ((addr & ~PAGE_MASK) || addr > USER_VM_SIZE || len > USER_VM_SIZE-addr) {
        return -1;
    }

    /* len is 0, nothing to do */
    if ((len = PAGE_ALIGN(len)) == 0) {
        return 0;
    }

    /* find the vma, addr < vma->m_end */
    vm_area_t* prev = NULL;
    vm_area_t* vma = find_vma(addr, prev);
    if (vma == NULL) { return -1;
    }

    /* make sure m_start <= addr< addr+len <= m_end */
    if (addr < vma->m_start || addr+len > vma->m_end) {
        return -1;
    }

    /* alloc a new vma, because the vma may split to 2 vma, such as:
     * [start, addr, addr+len, end] => [start, addr], [addr+len, end] */
    vm_area_t* vma_new = (vm_area_t*) os()->get_obj_pool(VMA_POOL)->alloc_from_pool();
    if (vma_new == NULL) {
        return -1;
    }

    /* set up the new vma and link to list */
    vma_new->m_start = addr+len;
    vma_new->m_end = vma->m_end;
    vma->m_end = addr;
    vma_new->m_next = vma->m_next;
    vma->m_next = vma_new;

    /* check if first part need to remove */
    if (vma->m_start == vma->m_end) {
        remove_vma(vma, prev);
        vma = prev;
    }

    /* check if second part need to remove */
    if (vma_new->m_start == vma_new->m_end) {
        remove_vma(vma_new, vma);
    }

    /* need to unmap the physical page */
    free_page_range(addr, addr+len);

    return 0;
}

uint32 vmm_t::do_protection_fault(vm_area_t* vma, uint32 addr, uint32 write)
{
    uint32 pa = os()->get_mm()->va_2_pa((void *) addr);
    if (pa == -1) {
        console()->kprintf(RED, "protection fault, no physical page found\n");
        return -1;
    }

    //console()->kprintf(YELLOW, "handle protection fault, %s, addr: %x, ref count: %u\n",
    //    current->m_name, addr, os()->get_mm()->get_page_ref(pa));

    // this is a write protection fault, but this vma can't write
    if (write && !(vma->m_flags & VM_WRITE)) {
        console()->kprintf(RED, "protection fault, ref count: %u!\n", 
                os()->get_mm()->get_page_ref(pa));
        return -1;
    }

    /* not shared */
    if (os()->get_mm()->get_page_ref(pa) == 1) {
        make_pte_write((void *) addr);
        return 0;
    }

    /* this page is shared, now only COW can share page */
    void* mem = os()->get_mm()->alloc_pages(0);
    os()->get_mm()->copy_page(mem, (void *) addr);

    os()->get_mm()->free_pages((void *) (PA2VA(pa)), 0);
    os()->get_mm()->map_pages(m_pg_dir, (void*) addr, VA2PA(mem), PAGE_SIZE, PTE_W | PTE_U);

    return 0;
}

void vmm_t::send_sig_segv()
{
    os()->get_process_mgr()->send_signal_to(current->m_pid, SIG_SEGV);
}

/*
 * bit 0: 0 no page found, 1 protection fault
 * bit 1: 0 read, 1 write
 * bit 2: 0 kernel, 1 user
 */
uint32 vmm_t::do_page_fault(trap_frame_t* frame)
{
    uint32 addr = 0xffffffff;
    __asm__ volatile("movl %%cr2, %%eax" : "=a" (addr));

    vm_area_t* vma = find_vma(addr);

    /* not find the vma or out of range */
    if (vma == NULL || vma->m_start > addr) {
        if (frame->err & 0x4) {
            if (vma != NULL && (vma->m_flags & VM_STACK) && addr + 32 >= frame->esp) {
                console()->kprintf(YELLOW, "expand stack\n");
                expand_stack(vma, addr);
                goto good_area;
            }
        }

        goto sig_segv;
    }

good_area:
    /* find a vma and the addr in this vma */
    if (vma != NULL && vma->m_start <= addr) {
        if (frame->err & 0x1) {
            return do_protection_fault(vma, addr, (uint32) (frame->err & 2));
        }

        /* no page found */
        void* mem = os()->get_mm()->alloc_pages(0);
        addr = (addr & PAGE_MASK);
        os()->get_mm()->map_pages(m_pg_dir, (void*) addr, VA2PA(mem), PAGE_SIZE, PTE_W | PTE_U);
    }

    return 0;

sig_segv:
    console()->kprintf(RED, "cpu: %u, process: %u, segment fault, addr: %x, err: %x, eip: %p, esp: %p\n",
        os()->get_arch()->get_current_cpu()->get_apic_id(), current->m_pid, addr, frame->err, frame->cs, frame->esp);

    //send_sig_segv();
    current->exit();
    return -1;
}

pde_t* vmm_t::get_pg_dir()
{
    return m_pg_dir;
}

void vmm_t::set_pg_dir(pde_t* pg_dir)
{
    m_pg_dir = pg_dir;
}

void vmm_t::make_pte_write(void* va)
{
    if ((uint32) va >= KERNEL_BASE) {
        return;
    }

    pde_t *pde = &m_pg_dir[PD_INDEX(va)];
    if (!(*pde & PTE_P)) {
        return;
    }

    pte_t* table = (pte_t *) PA2VA((*pde) & PAGE_MASK);
    if (!table[PT_INDEX(va)] & PTE_P) {
        return;
    }

    table[PT_INDEX(va)] |= PTE_W;
    asm volatile("movl %0, %%cr3": :"r" (VA2PA(m_pg_dir)));
}

uint32 vmm_t::expand_stack(vm_area_t* vma, uint32 addr)
{
    addr &= PAGE_MASK;
    uint32 grow = (vma->m_start - addr) >> PAGE_SHIFT;
    vma->m_start = addr;

    return 0;
}

void vmm_t::free_page_range(uint32 start, uint32 end)
{
    uint32 addr = start & PAGE_MASK;
    while (addr < end) {
        uint32 pa = os()->get_mm()->va_2_pa((void *) addr);
        if (pa != -1) {
            os()->get_mm()->free_pages((void *) (PA2VA(pa)), 0);
        }

        addr += PAGE_SIZE;
    }
}

void vmm_t::free_page_table()
{
    for (uint32 i = 0; i < KERNEL_BASE/(4*MB); i++) {
        pde_t *pde = &m_pg_dir[i];
        if (!(*pde & PTE_P)) {
            continue;
        }

        pte_t* table = (pte_t *) PA2VA((*pde) & PAGE_MASK);
        os()->get_mm()->free_pages(table, 0);
        *pde = 0;
    }

    asm volatile("movl %0, %%cr3": :"r" (VA2PA(m_pg_dir)));
}

void vmm_t::release()
{
    // 1. pages
    vm_area_t* vma = m_mmap;
    while (vma != NULL) {
        free_page_range(vma->m_start, vma->m_end);
        vma = vma->m_next;
    }

    // 2. page table
    free_page_table();

    // 3. vmas
    vma = m_mmap;
    while (vma != NULL) {
        vm_area_t* del = vma;
        vma = vma->m_next;
        os()->get_obj_pool(VMA_POOL)->free_object(del);
    }
    m_mmap = NULL;
}

