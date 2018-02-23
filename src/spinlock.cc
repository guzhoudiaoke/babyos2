/*
 * guzhoudiaoke@126.com
 * 2017-10-29
 */

#include "spinlock.h"
#include "x86.h"
#include "babyos.h"

spinlock_t::spinlock_t()
{
}
spinlock_t::~spinlock_t()
{
}

void spinlock_t::init()
{
    m_locked = 0;
}

uint32 spinlock_t::holding()
{
    return (m_locked == 1);
}

void spinlock_t::lock()
{
    while (xchg(&m_locked, 1) != 0)
        ;

    __sync_synchronize();
    wmb();
}

void spinlock_t::unlock()
{
    if (!holding()) {
        console()->kprintf(RED, "Not holding the lock when try to unlock\n");
        return;
    }

    __sync_synchronize();
    wmb();

    __asm__ volatile("movl $0, %0" : "+m" (m_locked));
}

void spinlock_t::lock_irq()
{
    cli();
    lock();
}

void spinlock_t::unlock_irq()
{
    unlock();
    sti();
}

void spinlock_t::lock_irqsave(uint32& flags)
{
    uint32 f;
    local_irq_save(f);
    lock();
    flags = f;
}

void spinlock_t::unlock_irqrestore(uint32 flags)
{
    unlock();
    local_irq_restore(flags);
}

/********************************************************************************************/

locker_t::locker_t(spinlock_t& lock) : m_lock(lock)
{
    m_lock.lock_irqsave(m_flags);
}

locker_t::~locker_t()
{
    m_lock.unlock_irqrestore(m_flags);
}

/********************************************************************************************/
