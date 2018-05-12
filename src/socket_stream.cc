/*
 * guzhoudiaoke@126.com
 * 2018-04-25
 */

#include "socket_stream.h"
#include "string.h"
#include "babyos.h"
#include "tcp.h"

spinlock_t      socket_stream_t::s_lock;
socket_stream_t  socket_stream_t::s_stream_sockets[MAX_STREAM_SOCKET];


void socket_stream_t::init_stream_sockets()
{
    socket_stream_t tmp;

    s_lock.init();
    for (int i = 0; i < MAX_STREAM_SOCKET; i++) {
        memcpy(&s_stream_sockets[i], &tmp, sizeof(socket_stream_t));
        s_stream_sockets[i].init();
    }
}

socket_t* socket_stream_t::alloc_stream_socket()
{
    locker_t locker(s_lock);

    socket_stream_t* socket = s_stream_sockets;
    for (int i = 0; i < MAX_STREAM_SOCKET; i++, socket++) {
        if (socket->m_ref == 0) {
            socket->m_ref = 1;
            return socket;
        }
    }

    return NULL;
}

void socket_stream_t::release_stream_socket(socket_t* socket)
{
}

socket_stream_t* socket_stream_t::lookup_stream_socket(sock_addr_inet_t* addr)
{
    locker_t locker(s_lock);

    socket_stream_t* socket = s_stream_sockets;
    for (int i = 0; i < MAX_STREAM_SOCKET; i++, socket++) {
        if (socket->m_addr == *addr) {
            return socket;
        }
    }

    return NULL;
}

int32 socket_stream_t::bind_stream_socket(socket_stream_t* socket, sock_addr_inet_t* addr)
{
    locker_t locker(s_lock);

    socket_stream_t* s = s_stream_sockets;
    for (int i = 0; i < MAX_STREAM_SOCKET; i++, s++) {
        if (s->m_addr == *addr && s->m_ref != 0) {
            return -1;
        }
    }

    socket->m_addr= *addr;
    return 0;
}

void socket_stream_t::get_not_used_port(socket_stream_t* socket)
{
    locker_t locker(s_lock);

    socket_stream_t* s = s_stream_sockets;
    for (uint16 port = 4096; port < 65535; port++) {
        bool succ = true;
        for (int i = 0; i < MAX_STREAM_SOCKET; i++, s++) {
            if (s->m_ref > 0 && s->m_addr.m_port == net_t::htons(port)) {
                succ = false;
                break;
            }
        }
        if (succ) {
            socket->m_addr.m_port = net_t::htons(port);
            return;
        }
    }
}

int32 socket_stream_t::stream_net_receive(tcp_hdr_t* hdr, net_buf_t* buf, uint32 src_ip, uint32 dst_ip)
{
    locker_t locker(s_lock);

    socket_stream_t* socket = s_stream_sockets;
    for (int i = 0; i < MAX_STREAM_SOCKET; i++, socket++) {
        if (socket->m_ref == 0) {
            continue;
        }

        //console()->kprintf(CYAN, "%p %u, %u\n", socket, socket->m_addr.m_port, hdr->m_dst_port);
        if (socket->m_addr.m_port == hdr->m_dst_port) {
            if (socket->net_receive(hdr, src_ip, net_t::ntohs(hdr->m_src_port), buf) == 0) {
                return 0;
            }
        }
    }

    console()->kprintf(RED, "not find socket_stream\n");
    return -1;
}

/**********************************************************************/

socket_stream_t::socket_stream_t()
{
    m_ref = 0;
}

void socket_stream_t::init()
{
    m_ref = 0;
}

int32 socket_stream_t::create(uint32 family, uint32 type, uint32 protocol)
{
    socket_t::create(family, type, protocol);
    if (protocol == 0) {
        protocol = socket_t::PROTO_TCP;
    }

    m_tcp_state = ST_CLOSED;
    m_ref = 1;
    m_addr.m_ip = 0;
    m_addr.m_port = 0;
    m_peer_addr.m_ip = 0;
    m_peer_addr.m_port = 0;
    m_buffer.init(this, 4);

    m_connecting_list.init(os()->get_obj_pool_of_size());
    m_sem.init(0);
    m_seq_no = 0;
    m_ack_no = 0;
    m_dup_socket = NULL;

    return 0;
}

int32 socket_stream_t::dup(socket_t* socket)
{
    create(socket->m_family, socket->m_type, socket->m_protocol);
    m_addr.m_ip = net_t::htonl(os()->get_net()->get_ipaddr());
    socket_stream_t::get_not_used_port(this);

    socket_stream_t* sock = (socket_stream_t *) socket;
    sock->m_dup_socket = this;
}

int32 socket_stream_t::get_addr(sock_addr_t* addr)
{
    *addr = m_addr;
    return 0;
}

int32 socket_stream_t::release()
{
    uint32 old_state = m_state;
    if (m_state != socket_t::SS_UNCONNECTED) {
        m_state = socket_t::SS_DISCONNECTING;
    }

    /* close tcp connect */
    return tcp_close();
}

int32 socket_stream_t::bind(sock_addr_t* myaddr)
{
    sock_addr_inet_t* addr = (sock_addr_inet_t *) myaddr;
    if (addr->m_ip == 0) {
        addr->m_ip = os()->get_net()->get_ipaddr();
    }

    return socket_stream_t::bind_stream_socket(this, addr);
}

int32 socket_stream_t::listen(uint32 backlog)
{
    /* nothing to do */
    m_tcp_state = ST_LISTEN;
    return 0;
}

int32 socket_stream_t::accept(socket_t* socket)
{
    socket_stream_t* server_socket = (socket_stream_t *) socket;
    if (server_socket->m_tcp_state != ST_LISTEN) {
        return -1;
    }

    /* wait for connect */
    while (server_socket->m_connecting_list.empty()) {
        server_socket->m_flags |= socket_t::SF_WAITDATA;
        server_socket->m_sem.down();
        server_socket->m_flags &= ~socket_t::SF_WAITDATA;
    }

    /* get a connect peer */
    spinlock_t* lock = server_socket->m_connecting_list.get_lock();
    uint32 flags;
    lock->lock_irqsave(flags);
    sock_addr_inet_t addr = *(server_socket->m_connecting_list.begin());
    server_socket->m_connecting_list.pop_front();
    lock->unlock_irqrestore(flags);

    m_peer_addr  = addr;
    console()->kprintf(CYAN, "%p %p, %u, peer ip: ", this, server_socket, m_ref);
    net_t::dump_ip_addr(m_peer_addr.m_ip);
    console()->kprintf(CYAN, " peer port: %u\n", m_peer_addr.m_port);

    /* set state to SYN_RCVD */
    m_tcp_state = ST_SYN_RCVD;
    console()->kprintf(CYAN, "SYN_RCVD\n");

    /* send SYN_ACK */
    tcp_ack(net_t::ntohl(m_addr.m_ip),net_t::ntohs(m_addr.m_port),
            net_t::ntohl(m_peer_addr.m_ip), net_t::ntohs(m_peer_addr.m_port), 
            m_ack_no, true);

    /* wait for ACK */
    m_sem.down();

    /* set state to ESTABLISHED */
    m_tcp_state = ST_ESTABLISHED;
    console()->kprintf(CYAN, "ESTABLISHED\n");

    /* set state to CONNECTED */
    m_state = socket_t::SS_CONNECTED;

    return 0;
}

int32 socket_stream_t::connect(sock_addr_t* server_addr)
{
    /* check server addr */
    if (server_addr->m_family != AF_INET) {
        return -EINVAL;
    }

    /* check socket state */
    if (m_state == SS_CONNECTING) {
        return -EINPROGRESS;
    }
    if (m_state == SS_CONNECTED) {
        return -EISCONN;
    }

    /* check tcp state */
    if (m_tcp_state != ST_CLOSED) {
        return -EINPROGRESS;
    }

    /* send SYN, then set state to SYN_SEND */
    m_addr.m_ip = net_t::htonl(os()->get_net()->get_ipaddr());
    socket_stream_t::get_not_used_port(this);
    tcp_syn((sock_addr_inet_t *) server_addr);
    m_tcp_state = ST_SYN_SENT;

    /* wait peer, when receive SYNC+ACK will wake up */
    m_sem.down();

    /* set tcp state to ESTABLISHED */
    m_tcp_state = ST_ESTABLISHED;

    /* set state to CONNECTED */
    m_state = SS_CONNECTED;

    /* send ACK */
    tcp_ack(net_t::ntohl(m_addr.m_ip), net_t::ntohs(m_addr.m_port),
            net_t::ntohl(m_peer_addr.m_ip), net_t::ntohs(m_peer_addr.m_port), m_ack_no, false);

    return 0;
}

int32 socket_stream_t::read(void* buf, uint32 size)
{
    return m_buffer.get_data((uint8 *) buf, size);
}

int32 socket_stream_t::write(void* buf, uint32 size)
{
    /* send data to peer */
    tcp_send(net_t::ntohl(m_addr.m_ip), net_t::ntohs(m_addr.m_port),
            net_t::ntohl(m_peer_addr.m_ip), net_t::ntohs(m_peer_addr.m_port), (uint8 *) buf, size);

    /* wait for ACK */
    m_sem.down();

    return size;
}

/* for now, socket stream not support */
int32 socket_stream_t::send_to(void *buf, uint32 size, sock_addr_t* addr_to)
{
    return -1;
}

int32 socket_stream_t::recv_from(void *buf, uint32 size, sock_addr_t* addr_from)
{
    return -1;
}

int32 socket_stream_t::tcp_syn(sock_addr_inet_t* dst_addr)
{
    console()->kprintf(YELLOW, "tcp syn\n");

    tcp_hdr_flag_t flag;
    flag.init();
    flag.m_syn = 1;
    return os()->get_net()->get_tcp()->transmit(net_t::ntohl(dst_addr->m_ip), 
            net_t::ntohs(m_addr.m_port), 
            net_t::ntohs(dst_addr->m_port), m_seq_no++, 0, flag, NULL, 0);

}

int32 socket_stream_t::tcp_ack(uint32 src_ip, uint16 src_port, uint32 dst_ip, uint16 dst_port, uint32 ack_no, bool syn)
{
    console()->kprintf(YELLOW, "tcp ack, ack no: %u, syn: %u\n", ack_no, syn ? 1 : 0);

    tcp_hdr_flag_t flag;
    flag.init();
    flag.m_ack = 1;
    flag.m_syn = syn ? 1 : 0;

    return os()->get_net()->get_tcp()->transmit(dst_ip, src_port, dst_port, m_seq_no++, ack_no, flag, NULL, 0);
}

int32 socket_stream_t::tcp_send(uint32 src_ip, uint16 src_port, uint32 dst_ip, uint16 dst_port, uint8* data, uint32 len)
{
    console()->kprintf(YELLOW, "tcp send to ip: \n");
    net_t::dump_ip_addr(dst_ip);
    console()->kprintf(YELLOW, " port: %u\n", dst_port);

    tcp_hdr_flag_t flag;
    flag.init();

    return os()->get_net()->get_tcp()->transmit(dst_ip, src_port, dst_port, m_seq_no++, 0, flag, data, len);
}

int32 socket_stream_t::tcp_fin(uint32 src_ip, uint16 src_port, uint32 dst_ip, uint16 dst_port, uint32 ack_no)
{
    tcp_hdr_flag_t flag;
    flag.init();
    flag.m_ack = 1;
    flag.m_fin = 1;

    return os()->get_net()->get_tcp()->transmit(dst_ip, src_port, dst_port, m_seq_no++, ack_no, flag, NULL, 0);
}

int32 socket_stream_t::net_receive_syn(tcp_hdr_t* hdr, uint32 src_ip, uint16 src_port)
{
    if (!hdr->m_flags.m_ack) {
        /* this is a SYN */
        if (m_tcp_state != ST_LISTEN) {
            return -1;
        }

        console()->kprintf(CYAN, "receive, SYN\n");

        sock_addr_inet_t addr;
        addr.m_ip = net_t::ntohl(src_ip);
        addr.m_port = net_t::ntohs(src_port);

        spinlock_t* lock = m_connecting_list.get_lock();
        uint32 flags;
        lock->lock_irqsave(flags);
        m_connecting_list.push_back(addr);
        lock->unlock_irqrestore(flags);


        socket_stream_t* dup = m_dup_socket;
        dup->m_ack_no = net_t::ntohl(hdr->m_seq_no) + 1;

        /* wake up server */
        m_sem.up();
    }
    else {
        console()->kprintf(CYAN, "receive, SYN ACK\n");

        /* this is a SYN_ACK */
        if (m_tcp_state != ST_SYN_SENT) {
            return -1;
        }

        /* save peer addr */
        m_peer_addr.m_ip = net_t::htonl(src_ip);
        m_peer_addr.m_port = net_t::htons(src_port);

        /* ack no */
        m_ack_no = net_t::ntohl(hdr->m_seq_no) + 1;

        /* wake up client */
        m_sem.up();
    }

    return 0;
}

int32 socket_stream_t::net_receive_ack(tcp_hdr_t* hdr, uint32 src_ip, uint16 src_port)
{
    console()->kprintf(CYAN, "receive, ACK no: %u\n", net_t::ntohl(hdr->m_ack_no));
    //console()->kprintf(CYAN, "%u, %u, %u, %u\n", net_t::ntohl(m_peer_addr.m_ip), src_ip,
    //         net_t::ntohs(m_peer_addr.m_port), src_port);
    if (net_t::ntohl(m_peer_addr.m_ip) != src_ip || net_t::ntohs(m_peer_addr.m_port) != src_port) {
        return -1;
    }

    //console()->kprintf(CYAN, "receive, ACK2, %u\n", m_tcp_state);
    if (m_tcp_state == ST_SYN_RCVD) {
        //console()->kprintf(CYAN, "receive, ACK3\n");
        m_sem.up();
        return 0;
    }
    else if (m_tcp_state == ST_ESTABLISHED) {
        //console()->kprintf(CYAN, "receive, ACK3\n");
        m_sem.up();
        return 0;
    }
    else if (m_tcp_state == ST_FIN_WAIT_1) {
        /* receive ACK */
        m_tcp_state = ST_FIN_WAIT_2;
        console()->kprintf(GREEN, "FIN_WAIT_2\n");
        return 0;
    }
    else if (m_tcp_state == ST_LAST_ACK) {
        /* receive ACK */
        m_tcp_state = ST_CLOSED;
        console()->kprintf(YELLOW, "CLOSED\n");
        return 0;
    }

    return 0;
}

int32 socket_stream_t::net_receive_data(tcp_hdr_t* hdr, net_buf_t* buf)
{
    console()->kprintf(CYAN, "receive data: %u, left: %u\n", buf->get_data_len(), m_buffer.get_free_space());
    if (m_buffer.put_data(buf->get_data(), buf->get_data_len()) < 0) {
        console()->kprintf(RED, "failed to put data.\n");
        return -1;
    }

    /* send ACK */
    m_ack_no = net_t::ntohl(hdr->m_seq_no) + 1;
    tcp_ack(net_t::ntohl(m_addr.m_ip), net_t::ntohs(m_addr.m_port),
            net_t::ntohl(m_peer_addr.m_ip), net_t::ntohs(m_peer_addr.m_port), 
            m_ack_no, false);

    return 0;
}

/* FIXME: should wake up process if it is waiting for read/write */
int32 socket_stream_t::net_receive_fin(tcp_hdr_t* hdr, uint32 src_ip, uint16 src_port)
{
    console()->kprintf(CYAN, "receive FIN\n");
    
    /* already CLOSED */
    if (m_tcp_state == ST_CLOSED) {
        return 0;
    }

    /* wake up socket, if waiting for FIN at FIN_WAIT_2 */
    if (m_tcp_state == ST_FIN_WAIT_2) {
        console()->kprintf(CYAN, "socke is FIN_WAIT_2, wake it up\n");
        m_sem.up();
        return 0;
    }

    /* have not connected */
    if (m_tcp_state != ST_ESTABLISHED) {
        /* FIXME: How about we are connecting ? */
        console()->kprintf(RED, "NOT connected\n");
        m_tcp_state = ST_CLOSED;
        return -1;
    }

    /* send ACK */
    m_ack_no = net_t::ntohl(hdr->m_seq_no) + 1;
    tcp_ack(net_t::ntohl(m_addr.m_ip), net_t::ntohs(m_addr.m_port),
            net_t::ntohl(m_peer_addr.m_ip), net_t::ntohs(m_peer_addr.m_port), m_ack_no, false);

    /* ESTABLISHED -> (receive FIN, send ACK) -> CLOSE_WAIT */
    m_tcp_state = ST_CLOSE_WAIT;
    console()->kprintf(YELLOW, "CLOSE_WAIT\n");

    /* send FIN */
    tcp_fin(net_t::ntohl(m_addr.m_ip), net_t::ntohs(m_addr.m_port),
            net_t::ntohl(m_peer_addr.m_ip), net_t::ntohs(m_peer_addr.m_port), m_ack_no);

    /* CLOSE_WAIT -> (send FIN) -> LAST_ACK */
    m_tcp_state = ST_LAST_ACK;
    console()->kprintf(YELLOW, "LAST_ACK\n");

    return 0;
}

int32 socket_stream_t::net_receive(tcp_hdr_t* hdr, uint32 src_ip, uint16 src_port, net_buf_t* buf)
{
    //console()->kprintf(GREEN, "socket_stream, receive a TCP packet from ip: ");
    //net_t::dump_ip_addr(src_ip);
    //console()->kprintf(GREEN, " port: %u, syn: %u, ack: %u, my ip ", net_t::ntohs(hdr->m_src_port),
    //        hdr->m_flags.m_syn, hdr->m_flags.m_ack);
    //net_t::dump_ip_addr(net_t::ntohl(m_addr.m_ip));
    //console()->kprintf(GREEN, " port: %u\n", net_t::ntohs(m_addr.m_port));

    m_ack_no = net_t::ntohl(hdr->m_seq_no) + 1;
    if (hdr->m_flags.m_syn) {
        /* SYN */
        return net_receive_syn(hdr, src_ip, src_port);
    }
    else if (hdr->m_flags.m_fin) {
        /* FIN */
        return net_receive_fin(hdr, src_ip, src_port);
    }
    else if (hdr->m_flags.m_ack) {
        /* ACK */
        return net_receive_ack(hdr, src_ip, src_port);
    }
    else {
        /* data */
        return net_receive_data(hdr, buf);
    }

    return 0;
}

int32 socket_stream_t::tcp_close()
{
    console()->kprintf(CYAN, "tcp close\n");

    /* not connected */
    if (m_tcp_state != ST_ESTABLISHED) {
        /* FIXME: how about connecting */
        m_tcp_state = ST_CLOSED;
        return -1;
    }

    /* send FIN */
    tcp_fin(net_t::ntohl(m_addr.m_ip), net_t::ntohs(m_addr.m_port),
            net_t::ntohl(m_peer_addr.m_ip), net_t::ntohs(m_peer_addr.m_port), m_ack_no);

    /* ESTABLISHED -> FIN_WAIT_1 */
    m_tcp_state = ST_FIN_WAIT_1;
    console()->kprintf(GREEN, "FIN_WAIT_1\n");

    /* wait for ACK */
    m_sem.down();

    /* receive FIN, send ACK */
    tcp_ack(net_t::ntohl(m_addr.m_ip), net_t::ntohs(m_addr.m_port),
            net_t::ntohl(m_peer_addr.m_ip), net_t::ntohs(m_peer_addr.m_port), m_ack_no, false);
    console()->kprintf(GREEN, "send ACK before CLOSED\n");
    
    /* FIXME: should have state TIME_WAIT, do not wait for now */
    m_tcp_state = ST_CLOSED;
    console()->kprintf(YELLOW, "CLOSED\n");

    /* free the socket */
    m_ref = 0;

    return 0;
}

