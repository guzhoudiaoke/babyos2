/*
 * 2017-12-28
 * guzhoudiaoke@126.com
 */

#include "signal.h"
#include "string.h"
#include "process.h"
#include "babyos.h"

signal_t::signal_t()
{
}

signal_t::~signal_t()
{
}

void signal_t::init()
{
    atomic_set(&m_count, 0);
    m_lock.init();

    for (int i = 0; i < NSIG; i++) {
        m_action[i].m_handler = SIG_DFL;
        m_action[i].m_mask = 0;
        m_action[i].m_flags = 0;
    }
}

void signal_t::copy(const signal_t& signal)
{
    atomic_set(&m_count, atomic_read(&signal.m_count));
    memcpy(m_action, signal.m_action, sizeof(sigaction_t) * NSIG);
    m_lock.init();
}

void signal_t::lock()
{
    m_lock.lock();
}

void signal_t::unlock()
{
    m_lock.unlock();
}

void signal_t::set_sigaction(uint32 sig, const sigaction_t& sa)
{
    m_action[sig].m_handler = sa.m_handler;
    m_action[sig].m_flags   = sa.m_flags;
    m_action[sig].m_mask    = sa.m_mask;
}

int32 signal_t::do_sigaction(uint32 sig, sighandler_t sig_handler)
{
    if (sig < 1 || sig >= NSIG) {
        return -1;
    }

    sigaction_t sa;
    sa.m_handler = sig_handler;

    current->m_signals.lock();
    current->m_signals.set_sigaction(sig, sa);
    current->m_signals.unlock();

    return 0;
}

int32 signal_t::do_send_signal(uint32 pid, uint32 sig)
{
    siginfo_t si;
    si.m_sig = sig;
    si.m_pid = current->m_pid;

    return os()->get_arch()->get_cpu()->send_signal_to(si, pid);
}

int32 signal_t::handle_signal(trap_frame_t* frame, const siginfo_t& si)
{
    uint32 sig = si.m_sig;
    sigaction_t* action = &current->m_signals.m_action[sig];
    if (action->m_handler == SIG_DFL) {
        return -1;
    }

    /* get sig frame */
    sigframe_t* sig_frame = (sigframe_t *) ((frame->esp - sizeof(sigframe_t)) & -8UL);
    sig_frame->m_sig = si.m_sig;

    /* save trap frame into sig frame */
    memcpy(&sig_frame->m_trap_frame, frame, sizeof(trap_frame_t));

    /* return addr */
    sig_frame->m_ret_addr = sig_frame->m_ret_code;

    /* return code: this is movl $,%eax ; int $0x80 */
    sig_frame->m_ret_code[0] = 0xb8;
    *(int *) (sig_frame->m_ret_code+1) = SYS_SIGRET;
    *(short *) (sig_frame->m_ret_code+5) = 0x80cd;

    /* set eip to sig handler */
    frame->eip = (uint32) action->m_handler;

    /* set esp as sig frame */
    frame->esp = (uint32) sig_frame;

    return 0;
}

int32 signal_t::do_sigreturn(trap_frame_t* frame)
{
    sigframe_t* sig_frame = (sigframe_t *) (frame->esp - 4);
    memcpy(frame, &sig_frame->m_trap_frame, sizeof(trap_frame_t));

    return 0;
}

