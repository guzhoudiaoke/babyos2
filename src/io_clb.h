/*
 * guzhoudiaoke@126.com
 * 2018-01-07
 */

#ifndef _IO_CLB_H_
#define _IO_CLB_H_

#include "types.h"
#include "kernel.h"
#include "process.h"

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

#endif
