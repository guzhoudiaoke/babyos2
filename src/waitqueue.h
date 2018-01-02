/*
 * 2018-01-02
 * guzhoudiaoke@126.com
 */

#ifndef _WAIT_QUEUE_H_
#define _WAIT_QUEUE_H_

#include "types.h"
#include "spinlock.h"
#include "list.h"

class process_t;
class wait_queue_t {
public:
    void init();
    void add(process_t* proc);
    void remove(process_t* proc);
    void wake_up();
private:
    spinlock_t              m_lock;
    list_t<process_t *>     m_procs;
};

#endif
