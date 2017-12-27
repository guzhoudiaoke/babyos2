/*
 * guzhoudiaoke@126.com
 * 2017-10-29
 */

#ifndef _HARDDISK_H_
#define _HARDDISK_H_

#include "types.h"
#include "spinlock.h"
#include "kernel.h"
#include "list.h"
#include "process.h"

#define HD_STATE_READY  0x40
#define HD_STATE_BUSY   0x80

#define IO_CMD_READ     0x20
#define IO_CMD_WRITE    0x30

#define IO_STATE_DONE   0x01

/* io control block */
typedef struct io_clb_s {
    uint32            flags;
    uint32            dev;
    uint32            read;
    uint32            lba;
    struct io_clb_s*  next;
    process_t*        wait;
    uint8             buffer[SECT_SIZE];

    void init(uint32 dev, uint32 read, uint32 lba) {
        this->flags = 0;
        this->dev = dev;
        this->read = read;
        this->lba = lba;
        this->wait = NULL;
    }
} io_clb_t;

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
    io_clb_t*   m_head;
    spinlock_t  m_lock;
};

#endif
