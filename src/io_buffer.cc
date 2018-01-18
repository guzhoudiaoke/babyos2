/*
 * guzhoudiaoke@126.com
 * 2018-01-17
 */

#include "io_buffer.h"


void io_buffer_t::init()
{
    m_lba = 0;
    m_done = 0;
    m_sem.init(1);
    m_sem_wait_done.init(0);
}

void io_buffer_t::lock()
{
    m_sem.down();
}

void io_buffer_t::unlock()
{
    m_sem.up();
}

void io_buffer_t::wait()
{
    m_sem_wait_done.down();
}

void io_buffer_t::done()
{
    m_sem_wait_done.up();
}

