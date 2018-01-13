/*
 * guzhoudiaoke@126.com
 * 2018-01-09
 */

#include "sem.h"

void sem_t::init(uint32 count)
{
    m_count = count;
    m_lock.init();
    m_wait_list.init();
}

void sem_t::down()
{
    m_lock.lock_irqsave();
    if (m_count > 0) {
        m_count--;
    }
    else {
        down_common();       
    }
    m_lock.unlock_irqrestore();
}

void sem_t::up()
{
    m_lock.lock_irqsave();
    if (m_wait_list.empty()) {
        m_count++;
    }
    else {
        up_common();
    }
    m_lock.unlock_irqrestore();
}

void sem_t::down_common()
{
    sem_waiter_t waiter;
    waiter.m_proc = current;
    waiter.m_up = false;

    m_wait_list.push_back(&waiter);

    while (true) {
        current->m_state = PROCESS_ST_SLEEP;
        m_lock.unlock_irqrestore();

        os()->get_arch()->get_cpu()->schedule();

        m_lock.lock_irqsave();
        if (waiter.m_up) {
            break;
        }
    }
}

void sem_t::up_common()
{
    sem_waiter_t* waiter = *m_wait_list.front();
    m_wait_list.pop_front();
    waiter->m_up = true;
    os()->get_arch()->get_cpu()->wake_up_process(waiter->m_proc);
}

