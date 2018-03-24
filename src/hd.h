/*
 * guzhoudiaoke@126.com
 * 2018-01-17
 */

#ifndef _HD_H_
#define _HD_H_

#include "types.h"
#include "list.h"
#include "spinlock.h"
#include "kernel.h"
#include "io_buffer.h"


#define HD_STATE_READY  0x40
#define HD_STATE_BUSY   0x80

#define IO_CMD_READ     0x20
#define IO_CMD_WRITE    0x30


class request_t {
public:
    void init(uint32 dev, uint32 lba, uint32 cmd, io_buffer_t* b);

public:
    enum {
        CMD_READ = 0,
        CMD_WRITE,
    };

    uint32          m_dev;
    uint32          m_lba;
    uint32          m_cmd;
    io_buffer_t*    m_buffer;
};


class hard_disk_t {
public:
    void init();
    void add_request(request_t* req);
    void do_irq();
    void wait();
    void do_request();
    void end_request();

private:
    spinlock_t          m_lock;
    list_t<request_t *> m_req_list;
    request_t*          m_current;
};

#endif
