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

    return 0;
}

