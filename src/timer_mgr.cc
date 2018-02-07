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
    lock->lock_irqsave();
    m_timer_list.push_back(timer);
    lock->unlock_irqrestore();
}

void timer_mgr_t::remove_timer(timer_t* timer)
{
    spinlock_t* lock = m_timer_list.get_lock();
    lock->lock_irqsave();
    list_t<timer_t*>::iterator it = m_timer_list.begin();
    while (it != m_timer_list.end()) {
        if (timer == *it) {
            m_timer_list.erase(it);
            break;
        }
    }
    lock->unlock_irqrestore();
}

