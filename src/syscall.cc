/*
 * guzhoudiaoke@126.com
 * 2017-10-30
 */

#include "syscall.h"
#include "babyos.h"
#include "ide.h"
#include "string.h"
#include "elf.h"
#include "x86.h"

extern int32 sys_print(trap_frame_t* frame);
extern int32 sys_fork(trap_frame_t* frame);
extern int32 sys_exec(trap_frame_t* frame);
static int32 (*system_call_table[])(trap_frame_t* frame) = {
    sys_print,
    sys_fork,
    sys_exec,
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
    // 1. read init from hd
    clb_elf.flags = 0;
    clb_elf.read = 1;
    clb_elf.dev = 0;
    clb_elf.lba = 512;

    // 2. read elf from hard disk
    uint8* buffer = (uint8*) os()->get_mm()->alloc_pages(3); // 8 pages, 32K
    for (uint8* b = buffer; b < buffer + 8*PAGE_SIZE; b += 512) {
        memset(clb_elf.buffer, 0, 512);
        os()->get_ide()->request(&clb_elf);
        memcpy(b, clb_elf.buffer, 512);

        clb_elf.flags = 0;
        clb_elf.lba++;
    }

    elf_hdr_t *elf = (elf_hdr_t *) (buffer);
    uint8 *base = (uint8 *) elf;

    // 3. check if it's a valid elf file
    if (elf->magic != ELF_MAGIC) {
        return -1;
    }

    pde_t* pg_dir = os()->get_mm()->get_pg_dir();

    // 4. load program segments
    prog_hdr_t *ph = (prog_hdr_t *)(base + elf->phoff);
    prog_hdr_t *end_ph = ph + elf->phnum;
    for (; ph < end_ph; ph++) {
        void* vaddr = (void*) (ph->vaddr & PAGE_MASK);
        uint32 offset = ph->vaddr - (uint32)vaddr;
        uint32 len = PAGE_ALIGN(ph->filesz + ((uint32)vaddr & (PAGE_SIZE-1)));
        uint32 pagenum = len / PAGE_SIZE;
        uint32 order = 0, num = 1;
        while (num < pagenum) {
            num *= 2;
            order++;
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

    // 6. eip & esp
    void* stack = os()->get_mm()->alloc_pages(1);
    os()->get_mm()->map_pages(pg_dir, (void*) USER_STACK_TOP-PAGE_SIZE*2, VA2PA(stack), PAGE_SIZE*2, PTE_W | PTE_U);
    frame->esp = USER_STACK_TOP;

    void (*entry)(void) = (void(*)(void))(elf->entry);
    frame->eip = (uint32)entry;         // need get eip by load elf and get address

    return 0;
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

