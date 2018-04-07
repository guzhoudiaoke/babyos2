/*
 * guzhoudiaoke@126.com
 * 2018-04-07
 */

#include "sock_addr.h"
#include "string.h"

bool sock_addr_local_t::operator == (const sock_addr_local_t& addr)
{
    return (m_family == addr.m_family) && (strcmp(m_path, addr.m_path) == 0);
}


bool sock_addr_inet_t::operator == (const sock_addr_inet_t& addr)
{
    return (m_family == addr.m_family) && (m_ip == addr.m_ip) && (m_port == addr.m_port);
}

