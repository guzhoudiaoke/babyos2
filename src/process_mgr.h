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
    void init();
    process_t* get_child_reaper();
    process_t* get_idle();
    process_t* find_process(uint32 pid);
    void release_process(process_t* proc);
    bool is_in_run_queue(process_t* proc);
    void add_process_to_rq(process_t* proc);
    void remove_process_from_list(process_t* proc);
    void add_process_to_list(process_t* proc);
    void wake_up_process(process_t* proc);

    spinlock_t* get_rq_lock();
    spinlock_t* get_proc_list_lock();

private:
    void init_idle_process();

private:
    process_t*		    m_idle_process;
    process_t*		    m_init_process;
    spinlock_t          m_rq_lock;
    list_t<process_t*>  m_proc_list;
};

#endif

