/*
 * guzhoudiaoke@126.com
 * 2018-01-05
 */

#ifndef _BUF_CACHE_H_
#define _BUF_CACHE_H_

#include "types.h"
#include "list.h"
#include "spinlock.h"
#include "io_clb.h"

typedef struct io_buffer_s {
    uint32      m_dev;
    uint32      m_lba;
    uint8       m_buffer[SECT_SIZE];
} io_buffer_t;

class buf_cache_t {
public:
    void        init();
    bool        read(io_clb_t* clb);
    void        insert(io_clb_t* clb);
    void        write(io_clb_t* clb);

private:
    io_buffer_t* find_cached_buffer(io_clb_t* clb);

private:
    uint32                 m_size;
    io_buffer_t*           m_bufs;
    list_t<io_buffer_t *>  m_used_list;
    list_t<io_buffer_t *>  m_free_list;
    spinlock_t             m_lock;
};

#endif

