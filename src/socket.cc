/*
 * guzhoudiaoke@126.com
 * 2018-01-20
 */

#include "socket.h"
#include "babyos.h"

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
    m_lock.lock();
    if (m_socket->m_state == socket_t::SS_CONNECTED) {
        ch = m_buffer[m_read_index];
        m_read_index = (m_read_index + 1) % SOCK_BUF_SIZE;
        ret = 0;
    }
    m_lock.unlock();
    m_wait_space.up();

    return ret;
}

int sock_buffer_t::put_char(char ch)
{
    int ret = -1;
    m_wait_space.down();
    m_lock.lock();
    if (m_socket->m_state == socket_t::SS_CONNECTED) {
        m_buffer[m_write_index] = ch;
        m_write_index = (m_write_index + 1) % SOCK_BUF_SIZE;
        ret = 0;
    }
    m_lock.unlock();
    m_wait_item.up();

    return ret;
}

/*********************************************************************/

int32 socket_t::create(uint32 family, uint32 type, uint32 protocol)
{
    m_family    = family;
    m_type      = type;
    m_protocol  = protocol;
    m_state     = SS_UNCONNECTED;

    m_connecting_list.init(os()->get_obj_pool_of_size());
    m_connected_socket = NULL;
    m_wait_connect_sem.init(0);
    m_wait_accept_sem.init(0);

    return 0;
}

