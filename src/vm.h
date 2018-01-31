/*
 * 2017-11-27
 * guzhoudiaoke@126.com
 */

#ifndef _VM_H_
#define _VM_H_

#include "types.h"
#include "traps.h"
#include "mm.h"

#define USER_VM_SIZE		(0xc0000000)
#define VM_UNMAPPED_BASE	(USER_VM_SIZE / 3)
#define USER_PTRS_PER_PGD   (USER_VM_SIZE / PGDIR_SIZE)


/* protect flags */
#define PROT_NONE           0x0       /* page can not be accessed */
#define PROT_READ           0x1       /* page can be read */
#define PROT_WRITE          0x2       /* page can be written */
#define PROT_EXEC           0x4       /* page can be executed */

/* map flags */
#define MAP_FIXED           0x10      /* Interpret addr exactly */

/* vm_flags.. */
#define VM_READ		        0x0001
#define VM_WRITE	        0x0002
#define VM_EXEC		        0x0004

#define VM_STACK	        0x0177


class vmm_t;
typedef struct vm_area_s {
	uint32		m_start;
	uint32		m_end;
	uint32		m_page_prot;
	uint32		m_flags;
	struct vm_area_s*	m_next;
} vm_area_t;


class vmm_t {
public:
	void init();
	uint32 copy(const vmm_t& vmm);

	uint32 do_mmap(uint32 addr, uint32 len, uint32 prot, uint32 flags);
	uint32 do_munmap(uint32 addr, uint32 len);

	/* Look up the first VMA which satisfies  addr < vm_end,  NULL if none. */
	vm_area_t* find_vma(uint32 addr);
	vm_area_t* find_vma(uint32 addr, vm_area_t*& prev);

	uint32 insert_vma(vm_area_t* vma);
	uint32 remove_vma(vm_area_t* vma, vm_area_t* prev);
	uint32 get_unmapped_area(uint32 len);
	uint32 do_page_fault(trap_frame_t* frame);

	pde_t* get_pg_dir();
	void   set_pg_dir(pde_t* pg_dir);

    void   release();

private:
	uint32 copy_page_table(pde_t* pg_dir);
	uint32 copy_vma(vm_area_t* mmap);
	uint32 do_protection_fault(vm_area_t* vma, uint32 addr, uint32 write);
    void   make_pte_write(void* va);
    uint32 expand_stack(vm_area_t* vma, uint32 addr);

    void   free_page_range(uint32 start, uint32 end);
    void   free_page_table();
    void   send_sig_segv();

private:
	vm_area_t*	m_mmap;
    pde_t*      m_pg_dir;

	///////
	uint32		m_code_start, m_code_end;
	uint32		m_data_start, m_data_end;
	uint32		m_brk_start, m_brk;
	uint32		m_stack_start;
	uint32		m_arg_start, m_arg_end;
	uint32		m_env_start, m_env_end;
};

#endif
