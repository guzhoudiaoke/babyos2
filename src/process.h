/*
 * guzhoudiaoke@126.com
 * 2017-12-27
 */

#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "types.h"
#include "list.h"
#include "vm.h"
#include "signal.h"
#include "waitqueue.h"
#include "fs.h"
#include "file.h"

#define MAX_OPEN_FILE 64

/* get current by kernel stack */
class process_t;
static inline process_t* get_current(void)
{
    process_t* current;
    __asm__("andl %%esp,%0; ":"=r" (current) : "0" (~8191UL));

    return current;
}
#define current get_current()


/* state of process */
enum {
    PROCESS_ST_RUNNING = 0,
    PROCESS_ST_SLEEP,
    PROCESS_ST_ZOMBIE,
};

typedef struct context_s {
    uint32	esp0;
    uint32	eip;
    uint32	esp;
    uint32	fs;
    uint32	gs;
} context_t;

class process_t {
public:
    process_t*  fork(trap_frame_t* frame);
    int32       exec(trap_frame_t* frame);
    void        sleep(uint32 ticks);
    void        sleep_on(wait_queue_t* queue);
    void        set_state(uint32 state);
    int32       exit();
    int32       wait_children(int32 pid);
    void        calc_sig_pending();

    int         alloc_fd(file_t* file);
    void        free_fd(int fd);
    file_t*     get_file(int fd);
    void        set_cwd(inode_t* inode);
    void        do_signal(trap_frame_t* frame);

    void        lock();
    void        unlock();

private:
    void        notify_parent();
    void        adope_children();

public:
    uint32              m_need_resched;
    uint32              m_sig_pending;
    char		        m_name[32];
    pid_t		        m_pid;
    uint32		        m_state;
    context_t	        m_context;
    vmm_t		        m_vmm;
    uint32              m_timeslice;

    process_t*          m_parent;
    list_t<process_t *> m_children;
    wait_queue_t        m_wait_child;

    signal_t            m_signal;
    list_t<siginfo_t>   m_sig_queue;
    sigset_t            m_sig_blocked;
    spinlock_t          m_sig_mask_lock;

    inode_t*            m_cwd;
    file_t*             m_files[MAX_OPEN_FILE];
    uint32              m_has_cpu;

    spinlock_t          m_task_lock;

    //process_t*	        m_prev;
    //process_t*	        m_next;
};

#endif

