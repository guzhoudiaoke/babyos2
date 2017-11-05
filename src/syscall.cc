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
extern int32 sys_sched(trap_frame_t* frame);
static int32 (*system_call_table[])(trap_frame_t* frame) = {
    sys_print,
    sys_fork,
    sys_exec,
    sys_sched,
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

int32 sys_exec(trap_frame_t* frame)
{
    frame->cs = (SEG_UCODE << 3 | 0x03);
    frame->ds = (SEG_UDATA << 3 | 0x03);
    frame->es = (SEG_UDATA << 3 | 0x03);
    frame->ss = (SEG_UDATA << 3 | 0x03);
    frame->fs = (SEG_UDATA << 3 | 0x03);
    frame->gs = (SEG_UDATA << 3 | 0x03);

    frame->eip = 0;
    frame->esp = 4096;

    return 0;
}

int32 sys_sched(trap_frame_t* frame)
{
    os()->get_arch()->get_cpu()->schedule();
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
        console()->kprintf(RED, "unknown system call %u\n", id);
        frame->eax = -1;
    }
    else {
        frame->eax = system_call_table[id](frame);
    }
}

