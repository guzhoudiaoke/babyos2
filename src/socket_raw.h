/*
 * guzhoudiaoke@126.com
 * 2018-04-07
 */

#ifndef _SOCKET_RAW_H_
#define _SOCKET_RAW_H_

#include "types.h"
#include "socket.h"
#include "net_buf.h"
#include "sem.h"

#define MAX_RAW_SOCKET    128


class socket_raw_t : public socket_t {
public:
    const uint32 c_max_buffer_num = 32;

    socket_raw_t();
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

    int32 net_receive(net_buf_t* buf);


    static void             init_raw_sockets();
    static socket_t*        alloc_raw_socket();
    static void             release_raw_socket(socket_t* socket);
    static socket_raw_t*    lookup_raw_socket(sock_addr_inet_t* addr);
    static int32            bind_raw_socket(socket_raw_t* socket, sock_addr_inet_t* addr);
    static int32            raw_net_receive(net_buf_t* buf, uint32 protocol, uint32 ip);

public:
    uint32              m_ref;
    sock_addr_inet_t    m_addr;
    sock_addr_inet_t    m_remote_addr;
    list_t<net_buf_t *> m_buffers;
    semaphore_t         m_buffer_sem;

    static spinlock_t   s_lock;
    static socket_raw_t s_raw_sockets[MAX_RAW_SOCKET];
};

#endif
