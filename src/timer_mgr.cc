/*
 * guzhoudiaoke@126.com
 * 2018-02-07
 */

#include "timer_mgr.h"
#include "babyos.h"

void timer_mgr_t::init()
{
    m_timer_list.init(os()->get_obj_pool_of_size());
}

void timer_mgr_t::update()
{
    list_t<timer_t*>::iterator it = m_timer_list.begin();
    while (it != m_timer_list.end()) {
        (*it)->update();
        it++;
    }
}

void timer_mgr_t::add_timer(timer_t* timer)
{
    spinlock_t* lock = m_timer_list.get_lock();
    uint32 flags;
    lock->lock_irqsave(flags);
    m_timer_list.push_back(timer);
    lock->unlock_irqrestore(flags);
}

void timer_mgr_t::remove_timer(timer_t* timer)
{
    spinlock_t* lock = m_timer_list.get_lock();
    uint32 flags;
    lock->lock_irqsave(flags);
    list_t<timer_t*>::iterator it = m_timer_list.find(timer);
    if (it != m_timer_list.end()) {
        m_timer_list.erase(it);
    }
    lock->unlock_irqrestore(flags);
}

