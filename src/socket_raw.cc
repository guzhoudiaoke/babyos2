/*
 * guzhoudiaoke@126.com
 * 2018-04-07
 */

#include "socket_raw.h"
#include "string.h"
#include "babyos.h"

spinlock_t      socket_raw_t::s_lock;
socket_raw_t    socket_raw_t::s_raw_sockets[MAX_RAW_SOCKET];

void socket_raw_t::init_raw_sockets()
{
    socket_raw_t tmp;

    s_lock.init();
    for (int i = 0; i < MAX_RAW_SOCKET; i++) {
        memcpy(&s_raw_sockets[i], &tmp, sizeof(socket_raw_t));
        s_raw_sockets[i].init();
    }
}

socket_t* socket_raw_t::alloc_raw_socket()
{
    locker_t locker(s_lock);

    socket_raw_t* socket = s_raw_sockets;
    for (int i = 0; i < MAX_RAW_SOCKET; i++, socket++) {
        if (socket->m_ref == 0) {
            socket->m_ref = 1;
            return socket;
        }
    }

    return NULL;
}

void socket_raw_t::release_raw_socket(socket_t* socket)
{
}

socket_raw_t* socket_raw_t::lookup_raw_socket(sock_addr_inet_t* addr)
{
    locker_t locker(s_lock);

    socket_raw_t* socket = s_raw_sockets;
    for (int i = 0; i < MAX_RAW_SOCKET; i++, socket++) {
        if (socket->m_addr == *addr) {
            return socket;
        }
    }

    return NULL;
}

int32 socket_raw_t::bind_raw_socket(socket_raw_t* socket, sock_addr_inet_t* addr)
{
    return -1;
}

/**********************************************************************/

socket_raw_t::socket_raw_t()
{
    m_ref = 0;
    m_addr;
    m_sock_buf;
}

void socket_raw_t::init()
{
}

int32 socket_raw_t::create(uint32 family, uint32 type, uint32 protocol)
{
    return -1;
}

int32 socket_raw_t::dup(socket_t* socket)
{
    return -1;
}

int32 socket_raw_t::get_name(sock_addr_t* addr)
{
    return -1;
}

int32 socket_raw_t::release()
{
    return -1;
}

int32 socket_raw_t::bind(sock_addr_t* myaddr)
{
    return -1;
}

int32 socket_raw_t::listen(uint32 backlog)
{
    return -1;
}

int32 socket_raw_t::accept(socket_t* server_socket)
{
    return -1;
}

int32 socket_raw_t::connect(sock_addr_t* server_addr)
{
    return -1;
}

int32 socket_raw_t::read(void* buf, uint32 size)
{
    return -1;
}

int32 socket_raw_t::write(void* buf, uint32 size)
{
    return -1;
}

int32 socket_raw_t::sendto(void *buf, uint32 size, uint32 flags, sock_addr_t* addr_to)
{
    return -1;
}

int32 socket_raw_t::recvfrom(void *buf, uint32 size, uint32 flags, sock_addr_t* addr_from)
{
    return -1;
}

