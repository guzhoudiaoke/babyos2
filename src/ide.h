/*
 * guzhoudiaoke@126.com
 * 2017-10-29
 */

#ifndef _HARDDISK_H_
#define _HARDDISK_H_

#include "types.h"
#include "spinlock.h"
#include "kernel.h"
#include "io_clb.h"
#include "buf_cache.h"

#define HD_STATE_READY  0x40
#define HD_STATE_BUSY   0x80

#define IO_CMD_READ     0x20
#define IO_CMD_WRITE    0x30

#define IO_STATE_DONE   0x01

class ide_t {
public:
    ide_t();
    ~ide_t();

    void init();
    void do_irq();
    void wait();
    void request(io_clb_t *clb);
    void start(io_clb_t *clb);

private:
    io_clb_t*       m_head;
    spinlock_t      m_lock;
    buf_cache_t     m_cache;
};

#endif
