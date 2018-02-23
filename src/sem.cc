/*
 * guzhoudiaoke@126.com
 * 2018-01-09
 */

#include "sem.h"
#include "babyos.h"
#include "process.h"

void semaphore_t::init(uint32 count)
{
    m_count = count;
    m_lock.init();
    m_wait_list.init(os()->get_obj_pool_of_size());
}

void semaphore_t::down()
{
    uint32 flags;
    m_lock.lock_irqsave(flags);
    if (m_count > 0) {
        m_count--;
        //console()->kprintf(WHITE, "%u_down_%p\t", os()->get_arch()->get_current_cpu()->get_apic_id(), this);
    }
    else {
        down_common();       
    }
    m_lock.unlock_irqrestore(flags);
}

void semaphore_t::up()
{
    uint32 flags;
    m_lock.lock_irqsave(flags);
    if (m_wait_list.empty()) {
        m_count++;
        //console()->kprintf(RED, "%u_up_%p\t", os()->get_arch()->get_current_cpu()->get_apic_id(), this);
    }
    else {
        up_common();
    }
    m_lock.unlock_irqrestore(flags);
}

void semaphore_t::down_common()
{
    sem_waiter_t waiter;
    waiter.m_proc = current;
    waiter.m_up = false;

    m_wait_list.push_back(&waiter);

    while (true) {
        current->m_state = PROCESS_ST_SLEEP;
        m_lock.unlock_irq();

        //console()->kprintf(RED, "%u_S_%u_%p\t", os()->get_arch()->get_current_cpu()->get_apic_id(), current->m_pid, this);
        os()->get_arch()->get_current_cpu()->schedule();

        m_lock.lock_irq();
        if (waiter.m_up) {
            break;
        }
    }
}

void semaphore_t::up_common()
{
    sem_waiter_t* waiter = *m_wait_list.begin();
    m_wait_list.pop_front();
    waiter->m_up = true;
    //console()->kprintf(YELLOW, "%u_Wake_%u_%p\t", os()->get_arch()->get_current_cpu()->get_apic_id(), waiter->m_proc->m_pid, this);
    os()->get_process_mgr()->wake_up_process(waiter->m_proc);
}

