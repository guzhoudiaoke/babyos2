/*
 * guzhoudiaoke@126.com
 * 2017-10-30
 */

#include "syscall.h"
#include "babyos.h"
#include "ide.h"
#include "string.h"

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

io_clb_t clb1;
int32 sys_exec(trap_frame_t* frame)
{
    // 1. read init from hd
    clb1.flags = 0;
    clb1.read = 1;
    clb1.dev = 0;
    clb1.lba = 512;

    memset(clb1.buffer, 0, 512);
    os()->get_ide()->request(&clb1);

    // 2. allocate a page and map to va 0-4k,
    pde_t* pg_dir = os()->get_mm()->get_pg_dir();

    void* mem = os()->get_mm()->alloc_pages(1);
    uint32* p = (uint32 *) 0;
    os()->get_mm()->map_pages(pg_dir, p, VA2PA(mem), 2*PAGE_SIZE, PTE_W | 0x04);

    // 3. load init to 0x0
    memcpy(p, clb1.buffer, 512);

    // frame
    frame->cs = (SEG_UCODE << 3 | 0x3);
    frame->ds = (SEG_UDATA << 3 | 0x3);
    frame->es = (SEG_UDATA << 3 | 0x3);
    frame->ss = (SEG_UDATA << 3 | 0x3);
    frame->fs = (SEG_UDATA << 3 | 0x3);
    frame->gs = (SEG_UDATA << 3 | 0x3);

    // eip & esp
    frame->eip = 0;         // need get eip by load elf and get address
    frame->esp = PAGE_SIZE*2;

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

