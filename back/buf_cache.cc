/*
 * guzhoudiaoke@126.com
 * 2018-01-05
 */

#include "buf_cache.h"
#include "babyos.h"
#include "string.h"


void buf_cache_t::init()
{
    m_lock.init();
    m_used_list.init();
    m_free_list.init();

    m_size = PAGE_SIZE * 64 / sizeof(io_buffer_t);
    m_bufs = (io_buffer_t *) os()->get_mm()->alloc_pages(6);
    for (int i = 0; i < m_size; i++) {
        m_free_list.push_back(m_bufs + i);
    }
}

io_buffer_t* buf_cache_t::find_cached_buffer(io_clb_t* clb)
{
    list_t<io_buffer_t *>::iterator it = m_used_list.begin();
    while (it != m_used_list.end()) {
        if ((*it)->m_dev == clb->dev && (*it)->m_lba == clb->lba) {
            break;
        }
        it++;
    }

    if (it != m_used_list.end()) {
        return *it;
    }
    return NULL;
}

bool buf_cache_t::read(io_clb_t* clb)
{
    locker_t locker(m_lock);
    io_buffer_t* buf = find_cached_buffer(clb);
    if (buf != NULL) {
        memcpy(clb->buffer, buf->m_buffer, SECT_SIZE);
        return true;
    }
    return false;
}

void buf_cache_t::insert(io_clb_t* clb)
{
    locker_t locker(m_lock);
    io_buffer_t* buf = find_cached_buffer(clb);
    if (buf != NULL) {
        return;
    }

    if (!m_free_list.empty()) {
        buf = *(m_free_list.begin());
        m_free_list.pop_front();
    }
    else {
        buf = *(m_used_list.begin());
        m_used_list.pop_front();
    }

    buf->m_dev = clb->dev;
    buf->m_lba = clb->lba;
    memcpy(buf->m_buffer, clb->buffer, SECT_SIZE);
    m_used_list.push_back(buf);
}

void buf_cache_t::write(io_clb_t* clb)
{
    locker_t locker(m_lock);
    io_buffer_t* buf = find_cached_buffer(clb);
    if (buf != NULL) {
        memcpy(buf->m_buffer, clb->buffer, SECT_SIZE);
    }
}

