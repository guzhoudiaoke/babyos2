/*
 * guzhoudiaoke@126.com
 * 2017-10-30
 */

#include "kernel.h"
#include "syscall.h"
#include "babyos.h"
#include "ide.h"
#include "string.h"
#include "elf.h"
#include "x86.h"

extern int32 sys_print(trap_frame_t* frame);
extern int32 sys_fork(trap_frame_t* frame);
extern int32 sys_exec(trap_frame_t* frame);
extern int32 sys_mmap(trap_frame_t* frame);
static int32 (*system_call_table[])(trap_frame_t* frame) = {
    sys_print,
    sys_fork,
    sys_exec,
    sys_mmap,
};

int32 sys_print(trap_frame_t* frame)
{
    // FIXME: need copy from user
    console()->kprintf(GREEN, "%s", frame->ebx);
}

int32 sys_fork(trap_frame_t* frame)
{
    process_t* proc = os()->get_arch()->get_cpu()->fork(frame);
    return proc->m_pid;
}

io_clb_t clb_elf;
int32 sys_exec(trap_frame_t* frame)
{
    uint32 lba = frame->ebx;
    uint32 sector_num = frame->ecx;

    // 1. read init from hd
    clb_elf.flags = 0;
    clb_elf.read = 1;
    clb_elf.dev = 0;
    clb_elf.lba = lba;

    // 2. read elf from hard disk
    uint8* buffer = (uint8*) os()->get_mm()->alloc_pages(3); // 8 pages, 32K
    for (uint8* b = buffer; b < buffer + SECT_SIZE*sector_num; b += SECT_SIZE) {
        memset(clb_elf.buffer, 0, SECT_SIZE);
        os()->get_ide()->request(&clb_elf);
        memcpy(b, clb_elf.buffer, SECT_SIZE);

        clb_elf.flags = 0;
        clb_elf.lba++;
    }

    elf_hdr_t *elf = (elf_hdr_t *) (buffer);
    uint8 *base = (uint8 *) elf;

    // 3. check if it's a valid elf file
    if (elf->magic != ELF_MAGIC) {
        return -1;
    }

    pde_t* pg_dir = current->m_vmm.get_pg_dir();

    // flush old mmap
    current->m_vmm.release();

    // 4. load program segments
    prog_hdr_t *ph = (prog_hdr_t *)(base + elf->phoff);
    prog_hdr_t *end_ph = ph + elf->phnum;
    for (; ph < end_ph; ph++) {
        if (ph->type != PT_LOAD || ph->filesz == 0) {
            continue;
        }
        void* vaddr = (void*) (ph->vaddr & PAGE_MASK);
        uint32 offset = ph->vaddr - (uint32)vaddr;
        uint32 len = PAGE_ALIGN(ph->filesz + ((uint32)vaddr & (PAGE_SIZE-1)));
        uint32 pagenum = len / PAGE_SIZE;
        uint32 order = 0, num = 1;
        while (num < pagenum) {
            num *= 2;
            order++;
        }

        int32 ret = current->m_vmm.do_mmap((uint32) vaddr, len, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_FIXED);
        if (ret < 0) {
            return -1;
        }
        void* mem = os()->get_mm()->alloc_pages(order);
        os()->get_mm()->map_pages(pg_dir, vaddr, VA2PA(mem), len, PTE_W | PTE_U);
        memcpy(mem+offset, base+ph->off, ph->filesz);
        if (ph->memsz > ph->filesz) {
            memset(mem+offset+ph->filesz, 0, ph->memsz - ph->filesz);
        }
    }

    // 5. frame
    frame->cs = (SEG_UCODE << 3 | 0x3);
    frame->ds = (SEG_UDATA << 3 | 0x3);
    frame->es = (SEG_UDATA << 3 | 0x3);
    frame->ss = (SEG_UDATA << 3 | 0x3);
    frame->fs = (SEG_UDATA << 3 | 0x3);
    frame->gs = (SEG_UDATA << 3 | 0x3);

    // 6. stack, esp
    vm_area_t* vma = (vm_area_t *) os()->get_obj_pool(VMA_POOL)->alloc_from_pool();
    if (vma == NULL) {
        return -1;
    }
    vma->m_end   = USER_STACK_TOP;
    vma->m_start = USER_STACK_TOP - PAGE_SIZE;
    vma->m_page_prot = PROT_READ | PROT_WRITE;
    vma->m_flags = VM_STACK;
    vma->m_next = NULL;
    if (current->m_vmm.insert_vma(vma) != 0) {
        return -1;
    }

    void* stack = os()->get_mm()->alloc_pages(0);
    os()->get_mm()->map_pages(pg_dir, (void*) USER_STACK_TOP-PAGE_SIZE, VA2PA(stack), PAGE_SIZE, PTE_W | PTE_U);
    frame->esp = USER_STACK_TOP-PAGE_SIZE;

    // 7. eip
    void (*entry)(void) = (void(*)(void))(elf->entry);
    frame->eip = (uint32)entry;         // need get eip by load elf and get address

    return 0;
}

int32 sys_mmap(trap_frame_t* frame)
{
    uint32 addr = frame->ebx, len = frame->ecx, prot = frame->edx, flags = frame->esi;
    addr = current->m_vmm.do_mmap(addr, len, prot, flags);
    return addr;
}

syscall_t::syscall_t()
{
}
syscall_t::~syscall_t()
{
}

void syscall_t::do_syscall(trap_frame_t* frame)
{
    uint32 id = frame->eax;
    if (id >= MAX_SYSCALL) {
        console()->kprintf(RED, "unknown system call %x, current: %p\n", id, current->m_pid);
        frame->eax = -1;
    }
    else {
        frame->eax = system_call_table[id](frame);
    }
}

