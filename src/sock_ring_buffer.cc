/*
 * guzhoudiaoke@126.com
 * 2018-04-07
 * separate from socket.h/c
 */

#include "sock_ring_buffer.h"
#include "socket.h"
#include "babyos.h"
#include "math.h"

void sock_ring_buffer_t::init(socket_t* socket, uint32 order)
{
    m_socket = socket;
    m_read_index = 0;
    m_write_index = 0;
    m_lock.init();
    m_wait_item.init(0);
    m_buffer = (uint8 *) os()->get_mm()->alloc_pages(order);
    m_length = PAGE_SIZE * math_t::pow(2, order);
    m_left = m_length;
    m_wait_space.init(m_length);
}

int32 sock_ring_buffer_t::get_char(uint8& ch)
{
    int ret = -1;
    m_wait_item.down();
    uint32 flags;
    m_lock.lock_irqsave(flags);
    if (m_socket->m_state == socket_t::SS_CONNECTED) {
        ch = m_buffer[m_read_index];
        m_read_index = (m_read_index + 1) % m_length;
        ret = 0;
        m_left++;
    }
    m_lock.unlock_irqrestore(flags);
    m_wait_space.up();

    return ret;
}

int32 sock_ring_buffer_t::put_char(uint8 ch)
{
    int ret = -1;
    m_wait_space.down();
    uint32 flags;
    m_lock.lock_irqsave(flags);
    if (m_socket->m_state == socket_t::SS_CONNECTED) {
        m_buffer[m_write_index] = ch;
        m_write_index = (m_write_index + 1) % m_length;
        ret = 0;
        m_left--;
    }
    m_lock.unlock_irqrestore(flags);
    m_wait_item.up();

    return ret;
}

uint32 sock_ring_buffer_t::get_free_space()
{
    return m_left;
}

int32 sock_ring_buffer_t::get_data(uint8* data, uint32 max)
{
    int32 ret = -1;
    m_wait_item.down();

    uint32 flags;
    m_lock.lock_irqsave(flags);
    if (m_socket->m_state == socket_t::SS_CONNECTED) {
        ret = 0;
        for (int i = 0; i < max; i++) {
            data[i] = m_buffer[m_read_index];
            m_read_index = (m_read_index + 1) % m_length;
            m_left++;
            ret++;
            m_wait_space.up();

            if (m_left == m_length) {
                break;
            }
            m_wait_item.down();
        }
    }
    m_lock.unlock_irqrestore(flags);

    return ret;
}

int32 sock_ring_buffer_t::put_data(uint8* data, uint32 len)
{
    uint32 flags;
    m_lock.lock_irqsave(flags);
    if (m_left < len) {
        console()->kprintf(RED, "%u %u no enough\n", m_left, len);
        m_lock.unlock_irqrestore(flags);
        return -1;
    }

    int32 ret = 0;
    if (m_socket->m_state == socket_t::SS_CONNECTED) {
        for (int i = 0; i < len; i++) {
            m_buffer[m_write_index] = data[i];
            m_write_index = (m_write_index + 1) % m_length;
            ret++;
            m_left--;
            m_wait_item.up();
            m_wait_space.down();
        }
    }
    m_lock.unlock_irqrestore(flags);

    return ret;
}

