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
}

void spinlock_t::unlock()
{
    if (!holding()) {
        console()->kprintf(RED, "Not holding the lock when try to unlock\n");
    }

    __sync_synchronize();

    //xchg(&m_locked, 0);
    m_locked = 0;
}

