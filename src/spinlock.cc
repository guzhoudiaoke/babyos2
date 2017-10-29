/*
 * guzhoudiaoke@126.com
 * 2017-10-29
 */

#include "spinlock.h"
#include "x86.h"
#include "babyos.h"

Spinlock::Spinlock()
{
}
Spinlock::~Spinlock()
{
}

void Spinlock::init()
{
    m_locked = 0;
}

uint32 Spinlock::holding()
{
    return (m_locked == 1);
}

void Spinlock::lock()
{
    while (xchg(&m_locked, 1) != 0)
        ;

    __sync_synchronize();
}

void Spinlock::unlock()
{
    if (!holding()) {
        os()->get_console()->kprintf(RED, "Not holding the lock when try to unlock\n");
    }

    __sync_synchronize();

    //xchg(&m_locked, 0);
    m_locked = 0;
}

