/*
 * 2018-01-02
 * guzhoudiaoke@126.com
 */

#include "waitqueue.h"
#include "process.h"
#include "babyos.h"

void wait_queue_t::init()
{
    m_lock.init();
    m_procs.init(os()->get_obj_pool_of_size());
}

void wait_queue_t::add(process_t* proc)
{
    m_lock.lock_irqsave();
    m_procs.push_back(proc);
    m_lock.unlock_irqrestore();
}

void wait_queue_t::remove(process_t* proc)
{
    m_lock.lock_irqsave();
    list_t<process_t *>::iterator it = m_procs.begin();
    while (it != m_procs.end()) {
        if (*it == proc) {
            m_procs.erase(it);
            break;
        }
        it++;
    }
    m_lock.unlock_irqrestore();
}

void wait_queue_t::wake_up()
{
    m_lock.lock_irqsave();
    if (!m_procs.empty()) {
        list_t<process_t *>::iterator it = m_procs.begin();
        os()->get_arch()->get_cpu()->wake_up_process((*it));
    }
    m_lock.unlock_irqrestore();
}

