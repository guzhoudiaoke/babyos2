/*
 * guzhoudiaoke@126.com
 * 2018-04-09
 */

#include "socket_dgram.h"
#include "string.h"
#include "babyos.h"

spinlock_t      socket_dgram_t::s_lock;
socket_dgram_t  socket_dgram_t::s_dgram_sockets[MAX_DGRAM_SOCKET];



void dgram_buf_t::init(uint32 ip, uint32 port, net_buf_t* buf)
{
    m_addr_from.m_family = socket_t::AF_INET;
    m_addr_from.m_ip = ip;
    m_addr_from.m_port = port;
    m_buf = buf;
}

dgram_buf_t& dgram_buf_t::operator = (const dgram_buf_t& dgram_buf)
{
    m_addr_from.m_family = dgram_buf.m_addr_from.m_family;
    m_addr_from.m_ip = dgram_buf.m_addr_from.m_ip;
    m_addr_from.m_port = dgram_buf.m_addr_from.m_port;
    m_buf = dgram_buf.m_buf;
}

/****************************************************************************/

void socket_dgram_t::init_dgram_sockets()
{
    socket_dgram_t tmp;
    tmp.init();

    s_lock.init();
    for (int i = 0; i < MAX_DGRAM_SOCKET; i++) {
        memcpy(&s_dgram_sockets[i], &tmp, sizeof(socket_dgram_t));
    }
}

socket_t* socket_dgram_t::alloc_dgram_socket()
{
    locker_t locker(s_lock);

    socket_dgram_t* socket = s_dgram_sockets;
    for (int i = 0; i < MAX_DGRAM_SOCKET; i++, socket++) {
        if (socket->m_ref == 0) {
            socket->m_ref = 1;
            return socket;
        }
    }

    return NULL;
}

void socket_dgram_t::release_dgram_socket(socket_t* socket)
{
}

socket_dgram_t* socket_dgram_t::lookup_dgram_socket(sock_addr_inet_t* addr)
{
    return NULL;
}

int32 socket_dgram_t::bind_dgram_socket(socket_dgram_t* socket, sock_addr_inet_t* addr)
{
    locker_t locker(s_lock);

    socket_dgram_t* s = s_dgram_sockets;
    for (int i = 0; i < MAX_DGRAM_SOCKET; i++, s++) {
        if (s->m_ref != 0 && s->m_addr == *addr) {
            return -1;
        }
    }

    socket->m_addr.m_ip = net_t::ntohl(addr->m_ip);
    socket->m_addr.m_port = net_t::ntohs(addr->m_port);

    return 0;
}

int32 socket_dgram_t::dgram_net_receive(net_buf_t* buf, uint32 src_ip, uint16 src_port, uint32 dst_ip, uint16 dst_port)
{
    locker_t locker(s_lock);

    socket_dgram_t* socket = s_dgram_sockets;
    for (int i = 0; i < MAX_DGRAM_SOCKET; i++, socket++) {
        if (socket->m_ref == 0) {
            continue;
        }

        if (socket->m_addr.m_port == dst_port) {
            if (socket->net_receive(src_ip, src_port, buf)) {
                return 0;
            }
        }
    }

    return -1;
}

void socket_dgram_t::get_not_used_port(socket_dgram_t* socket)
{
    locker_t locker(s_lock);

    socket_dgram_t* s = s_dgram_sockets;
    for (uint16 port = 4096; port < 65535; port++) {
        bool succ = true;
        for (int i = 0; i < MAX_DGRAM_SOCKET; i++, s++) {
            if (s->m_ref > 0 && s->m_addr.m_port == port) {
                succ = false;
                break;
            }
        }
        if (succ) {
            socket->m_addr.m_port = port;
            return;
        }
    }
}

/**********************************************************************/

socket_dgram_t::socket_dgram_t()
{
    m_ref = 0;
}

void socket_dgram_t::init()
{
}

int32 socket_dgram_t::create(uint32 family, uint32 type, uint32 protocol)
{
    socket_t::create(family, type, protocol);
    if (protocol == 0) {
        protocol = socket_t::PROTO_UDP;
    }

    m_ref = 1;
    m_addr.m_ip = 0;
    m_addr.m_port = 0;
    m_buffers.init(os()->get_obj_pool_of_size());
    m_buffer_sem.init(0);

    return 0;
}

int32 socket_dgram_t::dup(socket_t* socket)
{
    return -1;
}

int32 socket_dgram_t::get_addr(sock_addr_t* addr)
{
    return -1;
}

int32 socket_dgram_t::release()
{
    /* free all buffers */
    uint32 flag;
    spinlock_t* lock = m_buffers.get_lock();
    lock->lock_irqsave(flag);

    while (!m_buffers.empty()) {
        dgram_buf_t* dgram_buf = &(*m_buffers.begin());
        net_buf_t* buffer = dgram_buf->m_buf;
        os()->get_net()->free_net_buffer(buffer);
        m_buffers.pop_front();
    }

    lock->unlock_irqrestore(flag);

    /* set ref to 0 */
    m_ref = 0;
    return 0;
}

int32 socket_dgram_t::bind(sock_addr_t* myaddr)
{
    sock_addr_inet_t* addr = (sock_addr_inet_t *) myaddr;
    if (addr->m_ip == 0) {
        addr->m_ip = os()->get_net()->get_ipaddr();
    }

    return socket_dgram_t::bind_dgram_socket(this, addr);
}

int32 socket_dgram_t::listen(uint32 backlog)
{
    return -1;
}

int32 socket_dgram_t::accept(socket_t* server_socket)
{
    return -1;
}

int32 socket_dgram_t::connect(sock_addr_t* server_addr)
{
    return -1;
}

int32 socket_dgram_t::read(void* buf, uint32 size)
{
    return -1;
}

int32 socket_dgram_t::write(void* buf, uint32 size)
{
    return -1;
}

int32 socket_dgram_t::send_to(void *buf, uint32 size, sock_addr_t* addr_to)
{
    sock_addr_inet_t* addr = (sock_addr_inet_t *) addr_to;

    if (net_t::ntohs(m_addr.m_port) == 0) {
        m_addr.m_ip = os()->get_net()->get_ipaddr();
        socket_dgram_t::get_not_used_port(this);
    }

    os()->get_net()->get_udp()->transmit(net_t::ntohl(addr->m_ip), m_addr.m_port,
            net_t::ntohs(addr->m_port), (uint8 *) buf, size);

    return 0;
}

int32 socket_dgram_t::recv_from(void *buf, uint32 size, sock_addr_t* addr_from)
{
    m_buffer_sem.down();

    uint32 flag;
    spinlock_t* lock = m_buffers.get_lock();
    lock->lock_irqsave(flag);

    if (!m_buffers.empty()) {
        dgram_buf_t* dgram_buf = &(*m_buffers.begin());

        sock_addr_inet_t* addr = (sock_addr_inet_t *) addr_from;
        addr->m_family = net_t::ntohs(dgram_buf->m_addr_from.m_family);
        addr->m_ip = net_t::ntohl(dgram_buf->m_addr_from.m_ip);
        addr->m_port = net_t::ntohs(dgram_buf->m_addr_from.m_port);

        net_buf_t* buffer = dgram_buf->m_buf;
        uint32 data_len = buffer->get_data_len();
        uint8* data = buffer->get_data();

        if (size > data_len) {
            size = data_len;
        }

        memcpy(buf, data, size);
        os()->get_net()->free_net_buffer(buffer);
        m_buffers.pop_front();
    }

    lock->unlock_irqrestore(flag);

    return 0;
}

int32 socket_dgram_t::net_receive(uint32 ip, uint16 port, net_buf_t* buf)
{
    if (m_buffers.size() >= c_max_buffer_num) {
        return -ENOMEM;
    }

    uint32 flag;
    spinlock_t* lock = m_buffers.get_lock();
    lock->lock_irqsave(flag);
    dgram_buf_t dgram_buf;
    dgram_buf.init(ip, port, buf);
    m_buffers.push_back(dgram_buf);
    lock->unlock_irqrestore(flag);

    m_buffer_sem.up();

    return 0;
}

