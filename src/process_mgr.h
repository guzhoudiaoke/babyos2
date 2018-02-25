/*
 * guzhoudiaoke@126.com
 * 2018-02-07
 * separate from class cpu_t
 */

#ifndef _PROCESS_MGR_H_
#define _PROCESS_MGR_H_

#include "types.h"
#include "process.h"
#include "list.h"
#include "spinlock.h"

class process_mgr_t {
public:
    void        init();
    process_t*  get_child_reaper();
    process_t*  find_process(uint32 pid);
    void        release_process(process_t* proc);
    void        add_process_to_rq(process_t* proc);
    void        remove_process_from_rq(process_t* proc);
    void        add_process_to_list(process_t* proc);
    void        wake_up_process(process_t* proc);
    int32       send_signal_to(uint32 pid, uint32 sig);

    spinlock_t* get_rq_lock();
    spinlock_t* get_proc_list_lock();
    list_t<process_t *>* get_run_queue();
    uint32      get_next_pid();
    void        dump_run_queue();

private:
	atomic_t	        m_next_pid;
    spinlock_t          m_pid_lock;

    process_t*		    m_child_reaper;
    spinlock_t          m_rq_lock;
    list_t<process_t*>  m_proc_list;
    list_t<process_t*>  m_run_queue;
};

#endif

