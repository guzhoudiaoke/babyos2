/*
 * guzhoudiaoke@126.com
 * 2017-10-29
 */

#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#include "types.h"


#define local_irq_save(x)	    __asm__ __volatile__("pushfl ; popl %0 ; cli":"=g" (x): /* no input */ :"memory")
#define restore_flags(x) 	    __asm__ __volatile__("pushl %0 ; popfl": /* no output */ :"g" (x):"memory", "cc")
#define local_irq_restore(x)	restore_flags(x)

class spinlock_t {
public:
    spinlock_t();
    ~spinlock_t();

    void init();
    uint32 holding();
    void lock();
    void unlock();

    void lock_irq();
    void unlock_irq();

    void lock_irqsave(uint32& flags);
    void unlock_irqrestore(uint32 flags);

private:
    uint32  m_locked;
    uint32  m_flags;
};

class locker_t {
public:
    locker_t(spinlock_t& lock);
    ~locker_t();

private:
    uint32      m_flags;
    spinlock_t& m_lock;
};

#endif
