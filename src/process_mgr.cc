/*
 * guzhoudiaoke@126.com
 * 2018-02-07
 * separate from class cpu_t
 */

#include "process_mgr.h"
#include "babyos.h"
#include "x86.h"
#include "string.h"
#include "mm.h"

extern uint8  kernel_stack[];

void process_mgr_t::init()
{
    atomic_set(&m_next_pid, 1);
    m_pid_lock.init();
    m_init_process = NULL;
    m_rq_lock.init();
    m_proc_list.init(os()->get_obj_pool_of_size());
    m_run_queue.init(os()->get_obj_pool_of_size());

    //init_idle_process();
}

//void process_mgr_t::init_idle_process()
//{
//    m_idle_process = (process_t *) kernel_stack;
//    m_idle_process->m_pid = os()->get_next_pid();
//
//    m_idle_process->m_state = PROCESS_ST_RUNNING;
//    memset(&m_idle_process->m_context, 0, sizeof(context_t));
//    strcpy(m_idle_process->m_name, "idle");
//
//    m_idle_process->m_context.esp0 = ((uint32)(&kernel_stack) + KSTACK_SIZE);
//    m_idle_process->m_timeslice = 2;
//    m_idle_process->m_need_resched = 0;
//
//    /* signal */
//    m_idle_process->m_sig_queue.init(os()->get_obj_pool_of_size());
//    m_idle_process->m_sig_pending = 0;
//    m_idle_process->m_signal.init();
//
//    /* make link */
//    //m_idle_process->m_next = m_idle_process;
//    //m_idle_process->m_prev = m_idle_process;
//
//    m_idle_process->m_vmm.init();
//    m_idle_process->m_vmm.set_pg_dir(os()->get_mm()->get_kernel_pg_dir());
//    m_idle_process->m_children.init(os()->get_obj_pool_of_size());
//    m_idle_process->m_wait_child.init();
//
//    m_proc_list.push_back(m_idle_process);
//    //m_run_queue.push_back(m_idle_process);
//
//    for (int i = 0; i < MAX_OPEN_FILE; i++) {
//        m_idle_process->m_files[i] = NULL;
//    }
//}

process_t* process_mgr_t::find_process(uint32 pid)
{
    process_t* p = NULL;
    list_t<process_t*>::iterator it = m_proc_list.begin();
    while (it != m_proc_list.end()) {
        if ((*it)->m_pid == pid) {
            p = *it;
            break;
        }
        it++;
    }

    return p;
}

process_t* process_mgr_t::get_child_reaper()
{
    /* if have not set init process, set it by idle process's child */
    if (m_init_process == NULL) {
        //m_init_process = *m_idle_process->m_children.begin();
        m_init_process = *os()->get_arch()->get_boot_processor()->get_idle()->m_children.begin();
    }

    return m_init_process;
}

//process_t* process_mgr_t::get_idle()
//{
//    return m_idle_process;
//}

void process_mgr_t::release_process(process_t* proc)
{
    list_t<process_t*>::iterator it = m_proc_list.begin();
    while (it != m_proc_list.end()) {
        if (*it == proc) {
            m_proc_list.erase(it);
            break;
        }
        it++;
    }

    os()->get_mm()->free_pages(proc->m_vmm.get_pg_dir(), 0);
    os()->get_mm()->free_pages(proc, 1);
}

void process_mgr_t::add_process_to_rq(process_t* proc)
{
    spinlock_t* lock = m_run_queue.get_lock();
    uint32 flags;
    lock->lock_irqsave(flags);
    list_t<process_t *>::iterator it = m_run_queue.find(proc);
    if (it == m_run_queue.end()) {
        m_run_queue.push_front(proc);
        //console()->kprintf(BLUE, "P%u\t", proc->m_pid);
        //console()->kprintf(CYAN, "%u_put_%u\t", 
        //    os()->get_arch()->get_current_cpu()->get_apic_id(), proc->m_pid);
    }
    lock->unlock_irqrestore(flags);
}

void process_mgr_t::remove_process_from_rq(process_t* proc)
{
    spinlock_t* lock = m_run_queue.get_lock();
    uint32 flags;
    lock->lock_irqsave(flags);
    list_t<process_t *>::iterator it = m_run_queue.find(proc);
    if (it == m_run_queue.end()) {
        os()->panic("removing proc from run queue not in run queue");
    }
    m_run_queue.erase(it);
    lock->unlock_irqrestore(flags);
}

spinlock_t* process_mgr_t::get_rq_lock()
{
    return &m_rq_lock;
}

spinlock_t* process_mgr_t::get_proc_list_lock()
{
    return m_proc_list.get_lock();
}

void process_mgr_t::add_process_to_list(process_t* proc)
{
    spinlock_t* lock = m_proc_list.get_lock();
    uint32 flags;
    lock->lock_irqsave(flags);
    m_proc_list.push_back(proc);
    lock->unlock_irqrestore(flags);
}

void process_mgr_t::wake_up_process(process_t* proc)
{
    proc->set_state(PROCESS_ST_RUNNING);
    add_process_to_rq(proc);
}

int32 process_mgr_t::send_signal_to(uint32 pid, uint32 sig)
{
    siginfo_t si;
    si.m_sig = sig;
    si.m_pid = current->m_pid;

    spinlock_t* lock = os()->get_process_mgr()->get_proc_list_lock();
    uint32 flags;
    lock->lock_irqsave(flags);
    process_t* p = os()->get_process_mgr()->find_process(pid);
    if (p != NULL) {
        p->m_sig_queue.push_back(si);
        p->calc_sig_pending();
    }
    lock->unlock_irqrestore(flags);

    return 0;
}

list_t<process_t *>* process_mgr_t::get_run_queue()
{
    return &m_run_queue;
}

uint32 process_mgr_t::get_next_pid()
{
    uint32 pid = 0;
    while (1) {
        pid = atomic_read(&m_next_pid);
        atomic_inc(&m_next_pid);
        if (find_process(pid) == NULL) {
            break;
        }
    }
    return pid;
}

void process_mgr_t::dump_run_queue()
{
    console()->kprintf(WHITE, "run queue: [ ");
    list_t<process_t *>::iterator it = m_run_queue.begin();
    while (it != m_run_queue.end()) {
        process_t* p = *it;
        console()->kprintf(WHITE, "%u, ", p->m_pid);
        it++;
    }
    console()->kprintf(WHITE, " ]  ");
}

