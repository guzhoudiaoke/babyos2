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
#include "timer.h"

char syscall_t::s_print_buffer[1024] = {0};
int32 (*syscall_t::s_system_call_table[])(trap_frame_t* frame) = {
    syscall_t::sys_print,
    syscall_t::sys_fork,
    syscall_t::sys_exec,
    syscall_t::sys_mmap,
    syscall_t::sys_exit,
    syscall_t::sys_wait,
    syscall_t::sys_sleep,
    syscall_t::sys_signal,
    syscall_t::sys_sigret,
    syscall_t::sys_kill,
};

int32 syscall_t::sys_print(trap_frame_t* frame)
{
    char* va = (char *) PA2VA(os()->get_mm()->va_2_pa((void *) frame->ebx));
    strcpy(s_print_buffer, va);
    console()->kprintf(GREEN, "%s", s_print_buffer);
}

int32 syscall_t::sys_fork(trap_frame_t* frame)
{
    process_t* proc = os()->get_arch()->get_cpu()->fork(frame);
    console()->kprintf(PINK, "sys_fork, eip: %p esp: %p pid: %p\n", frame->eip, frame->esp, current->m_pid);
    return proc->m_pid;
}

int32 syscall_t::sys_exec(trap_frame_t* frame)
{
    console()->kprintf(YELLOW, "sys_exec, eip: %p esp: %p pid: %p, name: %s\n", 
            frame->eip, frame->esp, current->m_pid, frame->edx);
    return current->exec(frame);
}

int32 syscall_t::sys_mmap(trap_frame_t* frame)
{
    uint32 addr = frame->ebx, len = frame->ecx, prot = frame->edx, flags = frame->esi;
    addr = current->m_vmm.do_mmap(addr, len, prot, flags);
    return addr;
}

int32 syscall_t::sys_exit(trap_frame_t* frame)
{
    return current->exit();
}

int32 syscall_t::sys_wait(trap_frame_t* frame)
{
    int32 pid = frame->ebx;
    return current->wait_children(pid);
}

int32 syscall_t::sys_sleep(trap_frame_t* frame)
{
    uint32 ticks = frame->ebx * HZ;
    current->sleep(ticks);
    return 0;
}

int32 syscall_t::sys_signal(trap_frame_t* frame)
{
    uint32 sig = frame->ebx;
    sighandler_t sig_handler = (sighandler_t) frame->ecx;
    return current->m_signal.do_sigaction(sig, sig_handler);
}

int32 syscall_t::sys_sigret(trap_frame_t* frame)
{
    return signal_t::do_sigreturn(frame);
}

int32 syscall_t::sys_kill(trap_frame_t* frame)
{
    uint32 pid = frame->ebx;
    uint32 sig = frame->ecx;
    return signal_t::do_send_signal(pid, sig);
}

void syscall_t::do_syscall(trap_frame_t* frame)
{
    uint32 id = frame->eax;
    if (id >= MAX_SYSCALL) {
        console()->kprintf(RED, "unknown system call %x, current: %p\n", id, current->m_pid);
        frame->eax = -1;
    }
    else {
        frame->eax = s_system_call_table[id](frame);
    }
}

