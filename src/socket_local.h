/*
 * guzhoudiaoke@126.com
 * 2018-01-20
 */

#ifndef _SOCKET_LOCAL_H_
#define _SOCKET_LOCAL_H_

#include "types.h"
#include "socket.h"
#include "spinlock.h"

#define MAX_LOCAL_SOCKET    128


class socket_local_t : public socket_t {
public:
    socket_local_t();
    void  init();

    int32 create(uint32 family, uint32 type, uint32 protocol);
    int32 get_addr(sock_addr_t* addr);
    int32 release();
    int32 dup(socket_t* socket);

    int32 bind(sock_addr_t* myaddr);
    int32 listen(uint32 backlog);
    int32 accept(socket_t* server_socket);
    int32 connect(sock_addr_t* user_addr);
    int32 read(void* buf, uint32 size);
    int32 write(void* buf, uint32 size);
    int32 send_to(void *buf, uint32 size, sock_addr_t* addr_to);
    int32 recv_from(void *buf, uint32 size, sock_addr_t* addr_from);


    static void             init_local_sockets();
    static socket_t*        alloc_local_socket();
    static void             release_local_socket(socket_t* socket);
    static socket_local_t*  lookup_local_socket(sock_addr_local_t* addr);
    static int32            bind_local_socket(socket_local_t* socket, sock_addr_local_t* addr);

public:
    uint32                    m_ref;
    sock_addr_local_t         m_addr;
    sock_ring_buffer_t        m_sock_buf;

    list_t<socket_local_t *>  m_connecting_list;
    socket_local_t*           m_connected_socket;
    semaphore_t               m_wait_connect_sem;
    semaphore_t               m_wait_accept_sem;

    static spinlock_t         s_lock;
    static socket_local_t     s_local_sockets[MAX_LOCAL_SOCKET];
};

#endif
