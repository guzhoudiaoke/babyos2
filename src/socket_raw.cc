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
    tmp.init();

    s_lock.init();
    for (int i = 0; i < MAX_RAW_SOCKET; i++) {
        memcpy(&s_raw_sockets[i], &tmp, sizeof(socket_raw_t));
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
    //locker_t locker(s_lock);

    //socket_raw_t* socket = s_raw_sockets;
    //for (int i = 0; i < MAX_RAW_SOCKET; i++, socket++) {
    //    if (socket->m_addr == *addr) {
    //        return socket;
    //    }
    //}

    return NULL;
}

int32 socket_raw_t::bind_raw_socket(socket_raw_t* socket, sock_addr_inet_t* addr)
{
    return -1;
}

int32 socket_raw_t::raw_net_receive(net_buf_t* buf, uint32 protocol, uint32 ip)
{
    locker_t locker(s_lock);

    //console()->kprintf(CYAN, "raw_net_receive: protocol: %u, ip: ");
    //net_t::dump_ip_addr(ip);
    //console()->kprintf(CYAN, "\n");

    socket_raw_t* socket = s_raw_sockets;
    for (int i = 0; i < MAX_RAW_SOCKET; i++, socket++) {
        if (socket->m_ref == 0) {
            continue;
        }

        //if (socket->m_protocol == protocol && socket->m_remote_addr.m_ip == ip) {
        if (socket->m_protocol == protocol) {
            if (socket->net_receive(buf)) {
                return 0;
            }
        }
    }

    return -1;
}

/**********************************************************************/

socket_raw_t::socket_raw_t()
{
    m_ref = 0;
}

void socket_raw_t::init()
{
}

int32 socket_raw_t::create(uint32 family, uint32 type, uint32 protocol)
{
    socket_t::create(family, type, protocol);
    console()->kprintf(PINK, "protocol: %u\n", m_protocol);

    m_ref = 1;
    m_buffers.init(os()->get_obj_pool_of_size());
    m_buffer_sem.init(0);

    return 0;
}

int32 socket_raw_t::dup(socket_t* socket)
{
    return -1;
}

int32 socket_raw_t::get_addr(sock_addr_t* addr)
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

int32 socket_raw_t::send_to(void *buf, uint32 size, sock_addr_t* addr_to)
{
    sock_addr_inet_t* addr = (sock_addr_inet_t *) addr_to;
    console()->kprintf(GREEN, "socket_raw_t: send_to: ");
    net_t::dump_ip_addr(addr->m_ip);
    console()->kprintf(GREEN, ", port: %u, proto: %u\n", addr->m_port, m_protocol);

    os()->get_net()->get_ip()->transmit(addr->m_ip, (uint8 *) buf, size, m_protocol);
    return 0;
}

int32 socket_raw_t::recv_from(void *buf, uint32 size, sock_addr_t* addr_from)
{
    memcpy(&m_remote_addr, addr_from, sizeof(sock_addr_inet_t));
    m_buffer_sem.down();

    uint32 flag;
    spinlock_t* lock = m_buffers.get_lock();
    lock->lock_irqsave(flag);

    if (!m_buffers.empty()) {
        net_buf_t* buffer = *m_buffers.begin();
        uint32 data_len = buffer->get_data_len();
        uint8* data = buffer->get_data();

        if (size > data_len) {
            memcpy(buf, data, data_len);
            m_buffers.pop_front();
        }
        else {
            memcpy(buf, data, size);
            buffer->pop_front(size);
        }
    }

    lock->unlock_irqrestore(flag);

    return 0;
}

int32 socket_raw_t::net_receive(net_buf_t* buf)
{
    if (m_buffers.size() >= c_max_buffer_num) {
        return -ENOMEM;
    }

    uint32 flag;
    spinlock_t* lock = m_buffers.get_lock();
    lock->lock_irqsave(flag);
    m_buffers.push_back(buf);
    lock->unlock_irqrestore(flag);

    m_buffer_sem.up();

    return 0;
}

