/*
 * guzhoudiaoke@126.com
 * 2018-01-20
 */

#include "pipe.h"
#include "babyos.h"

void pipe_t::init()
{
    m_read_index = 0;
    m_write_index = 0;
    m_lock.init();
    m_space.init(PIPE_BUF_SIZE);
    m_item.init(0);
    m_readable = true;
    m_writable = true;
}

int pipe_t::get_char(char& ch)
{
    int ret = -1;
    m_item.down();
    uint32 flags;
    m_lock.lock_irqsave(flags);
    if (m_readable) {
        ch = m_buffer[m_read_index];
        m_read_index = (m_read_index + 1) % PIPE_BUF_SIZE;
        ret = 0;
    }
    m_lock.unlock_irqrestore(flags);
    m_space.up();

    return ret;
}

int pipe_t::put_char(char ch)
{
    int ret = -1;
    m_space.down();
    uint32 flags;
    m_lock.lock_irqsave(flags);
    if (m_writable) {
        m_buffer[m_write_index] = ch;
        m_write_index = (m_write_index + 1) % PIPE_BUF_SIZE;
        ret = 0;
    }
    m_lock.unlock_irqrestore(flags);
    m_item.up();

    return ret;
}

int32 pipe_t::read(void* buf, uint32 size)
{
    char* p = (char *) buf;
    char ch;
    for (uint32 i = 0; i < size; i++) {
        if (get_char(ch) < 0) {
            return -1;
        }
        *p++ = ch;
    }

    return size;
}

int32 pipe_t::write(void* buf, uint32 size)
{
    char* p = (char *) buf;
    for (uint32 i = 0; i < size; i++) {
        if (put_char(*p++) < 0) {
            return -1;
        }
    }

    return size;
}

void pipe_t::close(bool write_end)
{
    uint32 flags;
    m_lock.lock_irqsave(flags);

    if (write_end) {
        m_writable = false;
        m_item.up();
    }
    else {
        m_readable = false;
        m_space.up();
    }

    if (!m_readable && !m_writable) {
        m_lock.unlock_irqrestore(flags);
        os()->get_obj_pool(PIPE_POOL)->free_object(this);
    }

    m_lock.unlock_irqrestore(flags);
}

