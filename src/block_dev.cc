/*
 * guzhoudiaoke@126.com
 * 2018-01-15
 */

#include "block_dev.h"
#include "babyos.h"
#include "x86.h"
#include "hd.h"


#define BUFFER_PAGES        64
#define BUFFER_PAGES_ORDER  6


/*********************************************************/

void block_dev_t::init(uint32 dev)
{
    m_lock.init();
    m_used_list.init(os()->get_obj_pool_of_size());
    m_free_list.init(os()->get_obj_pool_of_size());

    m_dev = dev;
    m_buf_num = PAGE_SIZE * BUFFER_PAGES / sizeof(io_buffer_t);
    m_bufs = (io_buffer_t *) os()->get_mm()->alloc_pages(BUFFER_PAGES_ORDER);
    for (int i = 0; i < m_buf_num; i++) {
        m_bufs[i].init();
        m_free_list.push_back(m_bufs + i);
    }
}

io_buffer_t* block_dev_t::get_block(uint32 lba)
{
    io_buffer_t* b = NULL;

    uint32 flags;
    m_lock.lock_irqsave(flags);

    /* first, find from used list, if find it means this block cached */
    list_t<io_buffer_t *>::iterator it = m_used_list.begin();
    while (it != m_used_list.end() && (*it)->m_lba != lba) {
        it++;
    }

    /* find */
    if (it != m_used_list.end()) {
        b = *it;
    }
    else {
        /* get one from free list */
        if (!m_free_list.empty()) {
            b = *(m_free_list.begin());
            m_free_list.pop_front();
        }
        else {
            /* get front from used list */
            b = *(m_used_list.begin());
            m_used_list.pop_front();
        }

        b->m_lba = lba;
        b->m_done = 0;
    }
    m_lock.unlock_irqrestore(flags);

    if (b != NULL) {
        b->lock();
    }
    return b;
}

void block_dev_t::release_block(io_buffer_t* b)
{
    uint32 flags;
    m_lock.lock_irqsave(flags);
    m_used_list.push_back(b);
    m_lock.unlock_irqrestore(flags);
    b->unlock();
}

io_buffer_t* block_dev_t::read(uint32 lba)
{
    io_buffer_t* b = get_block(lba);
    if (b->m_done) {
        return b;
    }

    request_t req;
    req.init(m_dev, lba, request_t::CMD_READ, b);
    os()->get_hd()->add_request(&req);

    b->wait();

    return b;
}

int block_dev_t::write(io_buffer_t* b)
{
    request_t req;
    req.init(m_dev, b->m_lba, request_t::CMD_WRITE, b);
    os()->get_hd()->add_request(&req);

    b->wait();

    return 0;
}

