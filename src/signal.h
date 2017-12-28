/*
 * 2017-12-28
 * guzhoudiaoke@126.com
 */

#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#include "types.h"
#include "spinlock.h"
#include "atomic.h"

typedef void (*sighandler_t) (int32);
typedef uint64 sigset_t;

#define NSIG    32
#define SIG_DFL  (sighandler_t)(-1)

typedef struct sigaction_s {
    sighandler_t    m_handler;
    uint64          m_flags;
    sigset_t        m_mask;
} sigaction_t;

typedef struct siginfo_s {
    uint32          m_sig;
    uint32          m_pid;
} siginfo_t;

typedef struct sigframe_s {
	char*           m_ret_addr;
	uint32          m_sig;
    trap_frame_t    m_trap_frame;
	char            m_ret_code[8];
} sigframe_t;

class signal_t {
public:
    signal_t();
    ~signal_t();
    void init();
    void copy(const signal_t& signal);

    void lock();
    void unlock();
    void set_sigaction(uint32 sig, const sigaction_t& sa);

    static int32 do_sigaction(uint32 sig, sighandler_t sig_handler);
    static int32 do_send_signal(uint32 pid, uint32 sig);
    static int32 do_sigreturn(trap_frame_t* frame);
    static int32 handle_signal(trap_frame_t* frame, const siginfo_t& si);

public:

private:
    atomic_t    m_count;
    sigaction_t m_action[NSIG];
    spinlock_t  m_lock;
};

#endif

