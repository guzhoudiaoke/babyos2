/*
 * 2017-11-27
 * guzhoudiaoke@126.com
 */

#include "babyos.h"
#include "vm.h"
#include "mm.h"
#include "x86.h"

void vmm_t::init()
{
	m_mmap = NULL;
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
		if (USER_VM_SIZE - len > addr) {
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
	console()->kprintf(YELLOW, "do_mmap: 0x%x, 0x%x, 0x%x, 0x%x\n", addr, len, prot, flags);

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
			return -1;
		}
	}
	else {
		addr = get_unmapped_area(len);
		if (addr == 0) {
			return -1;
		}
	}

	/* alloc a vma from pool */
	vm_area_t* vma = (vm_area_t*) os()->get_obj_pool(VMA_POOL)->alloc_from_pool();
	if (vma == NULL) {
		return -1;
	}

	/* setup vma */
	vma->m_start = addr;
	vma->m_end = addr + len;
	vma->m_flags = 0;
	vma->m_page_prot = prot;

	/* insert vma into list, and do merge */
	if (insert_vma(vma)) {
		return -1;
	}

	return addr;
}

void vmm_t::remove_vma(vm_area_t* vma, vm_area_t* prev)
{
	if (prev != NULL) {
		prev->m_next = vma->m_next;
	}
	else {
		m_mmap = vma->m_next;
	}
	os()->get_obj_pool(VMA_POOL)->free_object(vma);
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
	if (vma == NULL) {
		return -1;
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
	//unmap_page_range(addr, len);

	return 0;
}

uint32 vmm_t::do_page_fault(trap_frame_t* frame)
{
	uint32 addr = 0xffffffff;
	__asm__ volatile("movl %%cr2, %%eax" : "=a" (addr));
	console()->kprintf(RED, "do_page_fault, addr: %x\n", addr);

	addr = (addr & PAGE_MASK);

	vm_area_t* vma = find_vma(addr);
	if (vma == NULL) {
		if (frame->err & 0x4) {
			console()->kprintf(RED, "segment fault!\n");
		}
		return -1;
	}

	if (vma->m_start <= addr) {
        void* mem = os()->get_mm()->alloc_pages(0);
        pde_t* pg_dir = os()->get_mm()->get_pg_dir();
        os()->get_mm()->map_pages(pg_dir, (void*) addr, VA2PA(mem), PAGE_SIZE, PTE_W | PTE_U);
        console()->kprintf(GREEN, "alloc and map pages\n");
	}
    else {
		if (frame->err & 0x4) {
			console()->kprintf(RED, "segment fault!\n");
		}
		return -1;
    }

	return 0;
}

