/*
 * guzhoudiaoke@126.com
 * 2018-01-20
 */

#include "socket.h"
#include "babyos.h"

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

