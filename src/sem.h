/*
 * guzhoudiaoke@126.com
 * 2018-01-09
 */

#ifndef _SEM_H_
#define _SEM_H_

#include "types.h"
#include "list.h"
#include "spinlock.h"

class process_t;
typedef struct sem_waiter_s {
    process_t*  m_proc;
    bool        m_up;
} sem_waiter_t;

class semaphore_t {
public:
    void init(uint32 count);
    void down();
    void up();

private:
    void down_common();
    void up_common();

private:
    spinlock_t              m_lock;
    uint32                  m_count;
    list_t<sem_waiter_t*>   m_wait_list;
};

#endif
