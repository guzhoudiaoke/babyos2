/*
 * guzhoudiaoke@126.com
 * 2018-04-07
 * separate from socket.h/c
 */

#include "sock_buffer.h"
#include "socket.h"

void sock_buffer_t::init(socket_t* socket)
{
    m_socket = socket;
    m_read_index = 0;
    m_write_index = 0;
    m_lock.init();
    m_wait_space.init(SOCK_BUF_SIZE);
    m_wait_item.init(0);
}

int sock_buffer_t::get_char(char& ch)
{
    int ret = -1;
    m_wait_item.down();
    uint32 flags;
    m_lock.lock_irqsave(flags);
    if (m_socket->m_state == socket_t::SS_CONNECTED) {
        ch = m_buffer[m_read_index];
        m_read_index = (m_read_index + 1) % SOCK_BUF_SIZE;
        ret = 0;
    }
    m_lock.unlock_irqrestore(flags);
    m_wait_space.up();

    return ret;
}

int sock_buffer_t::put_char(char ch)
{
    int ret = -1;
    m_wait_space.down();
    uint32 flags;
    m_lock.lock_irqsave(flags);
    if (m_socket->m_state == socket_t::SS_CONNECTED) {
        m_buffer[m_write_index] = ch;
        m_write_index = (m_write_index + 1) % SOCK_BUF_SIZE;
        ret = 0;
    }
    m_lock.unlock_irqrestore(flags);
    m_wait_item.up();

    return ret;
}

