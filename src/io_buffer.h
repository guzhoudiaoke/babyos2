/*
 * guzhoudiaoke@126.com
 * 2018-01-17
 */

#ifndef _IO_BUFFER_H_
#define _IO_BUFFER_H_

#include "types.h"
#include "sem.h"
#include "process.h"
#include "kernel.h"


class io_buffer_t {
public:

    void init();
    void lock();
    void unlock();
    void wait();
    void done();

public:
    uint32          m_lba;
    uint32          m_done;
    semaphore_t     m_sem;
    semaphore_t     m_sem_wait_done;
    uint8           m_buffer[SECT_SIZE];
};


#endif
