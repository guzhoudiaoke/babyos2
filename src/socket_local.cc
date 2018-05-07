/*
 * guzhoudiaoke@126.com
 * 2018-01-20
 */

#include "socket_local.h"
#include "string.h"
#include "babyos.h"

spinlock_t      socket_local_t::s_lock;
socket_local_t  socket_local_t::s_local_sockets[MAX_LOCAL_SOCKET];

void socket_local_t::init_local_sockets()
{
    socket_local_t tmp;

    s_lock.init();
    for (int i = 0; i < MAX_LOCAL_SOCKET; i++) {
        memcpy(&s_local_sockets[i], &tmp, sizeof(socket_local_t));
        s_local_sockets[i].init();
    }
}

socket_t* socket_local_t::alloc_local_socket()
{
    locker_t locker(s_lock);

    socket_local_t* socket = s_local_sockets;
    for (int i = 0; i < MAX_LOCAL_SOCKET; i++, socket++) {
        if (socket->m_ref == 0) {
            socket->m_ref = 1;
            return socket;
        }
    }

    return NULL;
}

void socket_local_t::release_local_socket(socket_t* socket)
{
}

socket_local_t* socket_local_t::lookup_local_socket(sock_addr_local_t* addr)
{
    locker_t locker(s_lock);

    socket_local_t* socket = s_local_sockets;
    for (int i = 0; i < MAX_LOCAL_SOCKET; i++, socket++) {
        if (socket->m_addr == *addr) {
            return socket;
        }
    }

    return NULL;
}

int32 socket_local_t::bind_local_socket(socket_local_t* socket, sock_addr_local_t* addr)
{
    locker_t locker(s_lock);

    socket_local_t* s = s_local_sockets;
    for (int i = 0; i < MAX_LOCAL_SOCKET; i++, s++) {
        if (s->m_addr == *addr && s->m_ref != 0) {
            return -1;
        }
    }

    memcpy(&socket->m_addr, addr, sizeof(sock_addr_local_t));
    return 0;
}

/**********************************************************************/

socket_local_t::socket_local_t()
{
    m_ref = 0;
}

void socket_local_t::init()
{
    m_ref = 0;
}

int32 socket_local_t::create(uint32 family, uint32 type, uint32 protocol)
{
    socket_t::create(family, type, protocol);

    m_ref = 1;
    m_sock_buf.init(this, 1);
    memset(m_addr.m_path, 0, sizeof(m_addr.m_path));

    m_connecting_list.init(os()->get_obj_pool_of_size());
    m_connected_socket = NULL;
    m_wait_connect_sem.init(0);
    m_wait_accept_sem.init(0);

    return 0;
}

int32 socket_local_t::dup(socket_t* socket)
{
    return create(socket->m_family, socket->m_type, socket->m_protocol);
}

int32 socket_local_t::get_addr(sock_addr_t* addr)
{
    sock_addr_local_t* local_addr = (sock_addr_local_t *) addr;
    memcpy(local_addr, &m_addr, sizeof(sock_addr_local_t));

    return 0;
}

int32 socket_local_t::release()
{
    uint32 old_state = m_state;
    if (m_state != socket_t::SS_UNCONNECTED) {
        m_state = socket_t::SS_DISCONNECTING;
    }

    /* wake up sockets who are waiting for accept */
    spinlock_t* lock = m_connecting_list.get_lock();
    uint32 flags;
    lock->lock_irqsave(flags);
    list_t<socket_local_t *>::iterator it = m_connecting_list.begin();
    while (it != m_connecting_list.end()) {
        (*it)->m_wait_accept_sem.up();
    }
    lock->unlock_irqrestore(flags);

    /* set peer state */
    socket_local_t* peer = (socket_local_t *) m_connected_socket;
    if (old_state == socket_t::SS_CONNECTED && peer != NULL) {
        peer->m_state = socket_t::SS_DISCONNECTING;
    }

    m_ref--;
    if (peer != NULL) {
        peer->m_ref--;
    }

    return 0;
}

int32 socket_local_t::bind(sock_addr_t* myaddr)
{
    sock_addr_local_t* addr = (sock_addr_local_t *) myaddr;
    return socket_local_t::bind_local_socket(this, addr);
}

int32 socket_local_t::listen(uint32 backlog)
{
    /* nothing to do */
    return 0;
}

int32 socket_local_t::accept(socket_t* socket)
{
    socket_local_t* server_socket = (socket_local_t* ) socket;

    /* wait for connect */
    while (server_socket->m_connecting_list.empty()) {
        server_socket->m_flags |= socket_t::SF_WAITDATA;
        server_socket->m_wait_connect_sem.down();
        //console()->kprintf(YELLOW, "wait connect waked up\n");
        server_socket->m_flags &= ~socket_t::SF_WAITDATA;
    }

    /* get a connect */
    socket_local_t* client_socket = NULL;

    spinlock_t* lock = server_socket->m_connecting_list.get_lock();
    uint32 flags;
    lock->lock_irqsave(flags);
    client_socket = *(server_socket->m_connecting_list.begin());
    server_socket->m_connecting_list.pop_front();
    lock->unlock_irqrestore(flags);

    client_socket->m_connected_socket = this;
    client_socket->m_state = socket_t::SS_CONNECTED;

    this->m_connected_socket = client_socket;
    this->m_state = socket_t::SS_CONNECTED;
    this->m_ref++;

    /* wake up client that accepted */
    client_socket->m_wait_accept_sem.up();

    return 0;
}

int32 socket_local_t::connect(sock_addr_t* server_addr)
{
    /* check state */
    if (m_state == SS_CONNECTING) {
        return -EINPROGRESS;
    }
    if (m_state == SS_CONNECTED) {
        return -EISCONN;
    }

    /* check server addr */
    if (server_addr->m_family != AF_LOCAL) {
        return -EINVAL;
    }

    /* get server socket */
    socket_local_t* server_socket = socket_local_t::lookup_local_socket((sock_addr_local_t *) server_addr);
    if (server_socket == NULL || server_socket->m_state != socket_t::SS_UNCONNECTED) {
        return -EINVAL;
    }

    /* check server */
    if (!(server_socket->m_flags & SF_ACCEPTCON)) {
        return -EINVAL;
    }

    /* add this to server socket's connecting list */
    spinlock_t* lock = server_socket->m_connecting_list.get_lock();
    uint32 flags;
    lock->lock_irqsave(flags);
    server_socket->m_connecting_list.push_back(this);
    lock->unlock_irqrestore(flags);

    /* notify server there is a new connect */
    server_socket->m_wait_connect_sem.up();

    /* wait for server finish accept */
    m_wait_accept_sem.down();

    /* check if connect correctly */
    if (this->m_state != SS_CONNECTED) {
        return m_connected_socket ? -EINTR : -EACCES;
    }

    m_ref++;
    //console()->kprintf(PINK, "inc ref, socket %p ref: %u\n", this, m_ref);
    return 0;
}

int32 socket_local_t::read(void* buf, uint32 size)
{
    uint8* p = (uint8 *) buf;
    uint8 ch;
    for (uint32 i = 0; i < size; i++) {
        if (m_sock_buf.get_char(ch) < 0) {
            return -1;
        }
        *p++ = ch;
    }

    return size;
}

int32 socket_local_t::write(void* buf, uint32 size)
{
    sock_ring_buffer_t* connect_sock_buf = &((socket_local_t *) (m_connected_socket))->m_sock_buf;
    uint8* p = (uint8 *) buf;
    for (uint32 i = 0; i < size; i++) {
        if (connect_sock_buf->put_char(*p++) < 0) {
            return -1;
        }
    }

    return size;
}

/* for now, socket local not support */
int32 socket_local_t::send_to(void *buf, uint32 size, sock_addr_t* addr_to)
{
    return -1;
}

int32 socket_local_t::recv_from(void *buf, uint32 size, sock_addr_t* addr_from)
{
    return -1;
}

