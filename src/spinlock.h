/*
 * guzhoudiaoke@126.com
 * 2017-10-29
 */

#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#include "types.h"

class spinlock_t {
public:
    spinlock_t();
    ~spinlock_t();

    void init();
    uint32 holding();
    void lock();
    void unlock();

private:
    uint32  m_locked;
};

#endif
