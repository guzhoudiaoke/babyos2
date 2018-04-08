/*
 * guzhoudiaoke@126.com
 * 2017-10-30
 */

#include "kernel.h"
#include "syscall.h"
#include "babyos.h"
#include "string.h"
#include "x86.h"
#include "sys_socket.h"

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
    syscall_t::sys_open,
    syscall_t::sys_close,
    syscall_t::sys_read,
    syscall_t::sys_write,
    syscall_t::sys_link,
    syscall_t::sys_unlink,
    syscall_t::sys_mkdir,
    syscall_t::sys_mknod,
    syscall_t::sys_dup,
    syscall_t::sys_stat,
    syscall_t::sys_chdir,
    syscall_t::sys_pipe,
    syscall_t::sys_send_to,
    syscall_t::sys_recv_from,
    syscall_t::sys_socket,
};

int32 syscall_t::sys_print(trap_frame_t* frame)
{
    color_ref_t color = frame->ebx;

    char buffer[512] = {0};
    char* va = (char *) PA2VA(os()->get_mm()->va_2_pa((void *) frame->ecx));
    strcpy(buffer, va);
    console()->kprintf(color, "%s", buffer);

    return 0;
}

int32 syscall_t::sys_fork(trap_frame_t* frame)
{
    process_t* proc = current->fork(frame);
    return proc == NULL ? -1 : proc->m_pid;
}

int32 syscall_t::sys_exec(trap_frame_t* frame)
{
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
    return current->m_signal.do_sigreturn(frame);
}

int32 syscall_t::sys_kill(trap_frame_t* frame)
{
    uint32 pid = frame->ebx;
    uint32 sig = frame->ecx;
    return os()->get_process_mgr()->send_signal_to(pid, sig);
}

int32 syscall_t::sys_open(trap_frame_t* frame)
{
    const char* path = (const char *) frame->ebx;
    int32 mode = frame->ecx;
    return os()->get_fs()->do_open(path, mode);
}

int32 syscall_t::sys_close(trap_frame_t* frame)
{
    int32 fd = frame->ebx;
    return os()->get_fs()->do_close(fd);
}

int32 syscall_t::sys_read(trap_frame_t* frame)
{
    int32 fd = frame->ebx;
    char* buf = (char *) frame->ecx;
    uint32 size = frame->edx;
    return os()->get_fs()->do_read(fd, buf, size);
}

int32 syscall_t::sys_write(trap_frame_t* frame)
{
    int32 fd = frame->ebx;
    char* buf = (char *) frame->ecx;
    uint32 size = frame->edx;
    return os()->get_fs()->do_write(fd, buf, size);
}

int32 syscall_t::sys_link(trap_frame_t* frame)
{
    char* path_old = (char *) frame->ebx;
    char* path_new = (char *) frame->ecx;
    return os()->get_fs()->do_link(path_old, path_new);
}

int32 syscall_t::sys_unlink(trap_frame_t* frame)
{
    char* path = (char *) frame->ebx;
    return os()->get_fs()->do_unlink(path);
}

int32 syscall_t::sys_mkdir(trap_frame_t* frame)
{
    char* path = (char *) frame->ebx;
    return os()->get_fs()->do_mkdir(path);
}

int32 syscall_t::sys_mknod(trap_frame_t* frame)
{
    char* path = (char *) frame->ebx;
    int major = frame->ecx;
    int minor = frame->edx;
    return os()->get_fs()->do_mknod(path, major, minor);
}

int32 syscall_t::sys_dup(trap_frame_t* frame)
{
    int fd = frame->ebx;
    return os()->get_fs()->do_dup(fd);
}

int32 syscall_t::sys_stat(trap_frame_t* frame)
{
    int fd = frame->ebx;
    stat_t* st = (stat_t *) frame->ecx;
    return os()->get_fs()->do_stat(fd, st);
}

int32 syscall_t::sys_chdir(trap_frame_t* frame)
{
    const char* path = (const char *) frame->ebx;
    return os()->get_fs()->do_chdir(path);
}

int32 syscall_t::sys_pipe(trap_frame_t* frame)
{
    int* fd = (int *) frame->ebx;
    return os()->get_fs()->do_pipe(fd);
}

int32 syscall_t::sys_socket(trap_frame_t* frame)
{
    return sys_socket_t::do_sys_socket(frame);
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

int32 syscall_t::sys_send_to(trap_frame_t* frame)
{
    int32 fd            = frame->ebx;
    char* buf           = (char *) frame->ecx;
    uint32 size         = frame->edx;
    sock_addr_t* addr   = (sock_addr_t *) frame->esi;

    return os()->get_fs()->do_send_to(fd, buf, size, addr);
}

int32 syscall_t::sys_recv_from(trap_frame_t* frame)
{
    int32 fd            = frame->ebx;
    char* buf           = (char *) frame->ecx;
    uint32 size         = frame->edx;
    sock_addr_t* addr   = (sock_addr_t *) frame->esi;
    
    return os()->get_fs()->do_recv_from(fd, buf, size, addr);
}

