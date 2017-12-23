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
extern int32 sys_exit(trap_frame_t* frame);
extern int32 sys_wait_pid(trap_frame_t* frame);
static int32 (*system_call_table[])(trap_frame_t* frame) = {
    sys_print,
    sys_fork,
    sys_exec,
    sys_mmap,
    sys_exit,
    sys_wait_pid,
};


int32 sys_print(trap_frame_t* frame)
{
    // FIXME: need copy from user
    static char buffer[1024];
    memset(buffer, 0, 1024);
    strcpy(buffer, (char *) frame->ebx);
    console()->kprintf(GREEN, "%s", buffer);
    //console()->kprintf(GREEN, "%s", frame->ebx);
}

int32 sys_fork(trap_frame_t* frame)
{
    process_t* proc = os()->get_arch()->get_cpu()->fork(frame);
    console()->kprintf(PINK, "sys_fork, eip: %p esp: %p pid: %p\n", frame->eip, frame->esp, current->m_pid);
    return proc->m_pid;
}

int32 sys_exec(trap_frame_t* frame)
{
    console()->kprintf(YELLOW, "sys_exec, eip: %p esp: %p pid: %p, name: %s\n", 
            frame->eip, frame->esp, current->m_pid, frame->edx);
    return elf_t::load(frame);
}

int32 sys_mmap(trap_frame_t* frame)
{
    uint32 addr = frame->ebx, len = frame->ecx, prot = frame->edx, flags = frame->esi;
    addr = current->m_vmm.do_mmap(addr, len, prot, flags);
    return addr;
}

int32 sys_exit(trap_frame_t* frame)
{
    //console()->kprintf(RED, "current: %p, pid: %x is exiting\n", current, current->m_pid);
    //current->m_state = PROCESS_ST_ZOMBIE;
    //current->m_vmm.release();
    //os()->get_arch()->get_cpu()->schedule();
    return 0;
}

int32 sys_wait_pid(trap_frame_t* frame)
{
    return 0;
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

