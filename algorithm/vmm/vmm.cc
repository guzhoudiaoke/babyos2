#include <cstring>
#include <iostream>
#include <cassert>
#include "vmm.h"

using namespace std;

object_pool_t::object_pool_t(uint32 size)
{
	m_obj_size = size;
	m_available = 0;
	m_free_list = NULL;
}

void object_pool_t::free_object(void* obj)
{
	object_pool_obj_t* o = (object_pool_obj_t*) obj;
	o->m_next = NULL;
	if (m_free_list == NULL) {
		m_free_list = o;
	}
	else {
		o->m_next = m_free_list;
		m_free_list = o;
	}
	m_available++;
}

void* object_pool_t::alloc_from_pool()
{
	if (m_free_list == NULL) {
		unsigned char* mem = new unsigned char[4096];
		unsigned char* end = mem + 4096;
		while (mem + m_obj_size <= end) {
			free_object(mem);
			mem += m_obj_size;
		}
	}

	void* obj = m_free_list;
	m_free_list = m_free_list->m_next;

	m_available--;
	return obj;
}

uint32 object_pool_t::get_available()
{
	return m_available;
}

/* ------------------------------------------------------------------------------ */

vmm_t::vmm_t()
{
	m_vma_pool = new object_pool_t(sizeof(vm_area_t));
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
			m_vma_pool->free_object(vma);
			vma = prev;
		}
	}

	/* merge vma and p */
	if (p != NULL && vma->m_end == p->m_start) {
		if (vma->m_page_prot == p->m_page_prot && vma->m_flags == p->m_flags) {
			vma->m_end = p->m_end;
			vma->m_next = p->m_next;
			m_vma_pool->free_object(p);
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
	vm_area_t* vma = (vm_area_t*) m_vma_pool->alloc_from_pool();
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
	m_vma_pool->free_object(vma);
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
	vm_area_t* vma_new = (vm_area_t*) m_vma_pool->alloc_from_pool();
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

void vmm_t::dump()
{
	cout << endl;
	cout << "-----------------------------------" << endl;
	vm_area_t* p = m_mmap;
	while (p != NULL) {
		cout << "[" << p->m_start << ", " << p->m_end << "(" << p->m_page_prot << ")] -> ";
		p = p->m_next;
	}
	cout << endl;
}

object_pool_t* vmm_t::get_vma_pool()
{
	return m_vma_pool;
}

/*****************************************************************************/

void test1(vmm_t& vmm)
{
	assert(vmm.get_vma_pool()->get_available() == 0);

	// invalid addr
	assert(-1 == vmm.do_mmap(1234, 1024, 0, MAP_FIXED));

	// mmap
	assert(1*PAGE_SIZE == vmm.do_mmap(1*PAGE_SIZE, 1024, 0, MAP_FIXED));
	vm_area_t* vma = vmm.find_vma(1*PAGE_SIZE);
	assert(vma != NULL);
	assert(vma->m_start == 1*PAGE_SIZE);
	assert(vma->m_end == 2*PAGE_SIZE);
	assert(vmm.get_vma_pool()->get_available() == PAGE_SIZE/sizeof(vm_area_t) - 1);

	// merge1
	assert(2*PAGE_SIZE == vmm.do_mmap(2*PAGE_SIZE, 1024, 0, MAP_FIXED));
	vma = vmm.find_vma(1*PAGE_SIZE);
	assert(vma != NULL);
	assert(vma->m_start == 1*PAGE_SIZE);
	assert(vma->m_end == 3*PAGE_SIZE);
	assert(vmm.get_vma_pool()->get_available() == PAGE_SIZE/sizeof(vm_area_t) - 1);

	// mmap2
	assert(5*PAGE_SIZE == vmm.do_mmap(5*PAGE_SIZE, 1234, 0, MAP_FIXED));
	vma = vmm.find_vma(5*PAGE_SIZE);
	assert(vma != NULL);
	assert(vma->m_start == 5*PAGE_SIZE);
	assert(vma->m_end == 6*PAGE_SIZE);
	assert(vmm.get_vma_pool()->get_available() == PAGE_SIZE/sizeof(vm_area_t) - 2);

	// merge2
	assert(4*PAGE_SIZE == vmm.do_mmap(4*PAGE_SIZE, 1234, 0, MAP_FIXED));
	vma = vmm.find_vma(4*PAGE_SIZE);
	assert(vma != NULL);
	assert(vma->m_start == 4*PAGE_SIZE);
	assert(vma->m_end == 6*PAGE_SIZE);
	assert(vmm.get_vma_pool()->get_available() == PAGE_SIZE/sizeof(vm_area_t) - 2);

	// merge3
	assert(3*PAGE_SIZE == vmm.do_mmap(3*PAGE_SIZE, 1234, 0, MAP_FIXED));
	vma = vmm.find_vma(3*PAGE_SIZE);
	assert(vma != NULL);
	assert(vma->m_start == 1*PAGE_SIZE);
	assert(vma->m_end == 6*PAGE_SIZE);
	assert(vmm.get_vma_pool()->get_available() == PAGE_SIZE/sizeof(vm_area_t) - 1);

	// unmap invalid addr
	assert(-1 == vmm.do_munmap(123, 1024));

	// unmap tail
	assert(-1 != vmm.do_munmap(5*PAGE_SIZE, PAGE_SIZE));
	vma = vmm.find_vma(3*PAGE_SIZE);
	assert(vma != NULL);
	assert(vma->m_start == 1*PAGE_SIZE);
	assert(vma->m_end == 5*PAGE_SIZE);
	assert(vmm.get_vma_pool()->get_available() == PAGE_SIZE/sizeof(vm_area_t) - 1);

	// unmap head
	assert(-1 != vmm.do_munmap(1*PAGE_SIZE, PAGE_SIZE));
	vma = vmm.find_vma(3*PAGE_SIZE);
	assert(vma != NULL);
	assert(vma->m_start == 2*PAGE_SIZE);
	assert(vma->m_end == 5*PAGE_SIZE);
	assert(vmm.get_vma_pool()->get_available() == PAGE_SIZE/sizeof(vm_area_t) - 1);

	// unmap mid
	assert(-1 != vmm.do_munmap(3*PAGE_SIZE, PAGE_SIZE));
	vma = vmm.find_vma(2*PAGE_SIZE);
	assert(vma != NULL);
	assert(vma->m_start == 2*PAGE_SIZE);
	assert(vma->m_end == 3*PAGE_SIZE);
	vma = vmm.find_vma(4*PAGE_SIZE);
	assert(vma != NULL);
	assert(vma->m_start == 4*PAGE_SIZE);
	assert(vma->m_end == 5*PAGE_SIZE);
	assert(vmm.get_vma_pool()->get_available() == PAGE_SIZE/sizeof(vm_area_t) - 2);


	// mmap
	assert(VM_UNMAPPED_BASE == vmm.do_mmap(1*PAGE_SIZE, 1024, 0, 0));
	vma = vmm.find_vma(VM_UNMAPPED_BASE);
	assert(vma != NULL);
	assert(vma->m_start == VM_UNMAPPED_BASE);
	assert(vma->m_end == VM_UNMAPPED_BASE+1*PAGE_SIZE);
	assert(vmm.get_vma_pool()->get_available() == PAGE_SIZE/sizeof(vm_area_t) - 3);
}

int main()
{
	vmm_t vmm;
	test1(vmm);
	return 0;
}

