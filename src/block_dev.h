/*
 * guzhoudiaoke@126.com
 * 2018-01-15
 */

#ifndef _BLOCK_DEV_H_
#define _BLOCK_DEV_H_

#include "types.h"
#include "list.h"
#include "sem.h"
#include "spinlock.h"
#include "kernel.h"
#include "io_buffer.h"


class block_dev_t {
public:
    void             init(uint32 dev);
    io_buffer_t*     read(uint32 lba);
    int              write(io_buffer_t* b);
    void             release_block(io_buffer_t* b);

private:
    io_buffer_t* get_block(uint32 lba);

private:
    uint32                      m_dev;
    uint32                      m_buf_num;
    io_buffer_t*                m_bufs;
    list_t<io_buffer_t *>       m_used_list;
    list_t<io_buffer_t *>       m_free_list;
    spinlock_t                  m_lock;
};

#endif
