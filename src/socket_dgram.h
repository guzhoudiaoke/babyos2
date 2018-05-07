/*
 * guzhoudiaoke@126.com
 * 2018-04-09
 */

#ifndef _SOCKET_DGRAM_H_
#define _SOCKET_DGRAM_H_

#include "types.h"
#include "socket.h"
#include "net_buf.h"
#include "sem.h"

#define MAX_DGRAM_SOCKET    128

class dgram_buf_t {
public:
    sock_addr_inet_t m_addr_from;
    net_buf_t*       m_buf;

    void init(uint32 ip, uint32 port, net_buf_t* buf);
    dgram_buf_t& operator = (const dgram_buf_t& it);
};

class socket_dgram_t : public socket_t {
public:
    const uint32 c_max_buffer_num = 32;

    socket_dgram_t();
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

    int32 net_receive(uint32 ip, uint16 port, net_buf_t* buf);


    static void             init_dgram_sockets();
    static socket_t*        alloc_dgram_socket();
    static void             release_dgram_socket(socket_t* socket);
    static socket_dgram_t*  lookup_dgram_socket(sock_addr_inet_t* addr);
    static int32            bind_dgram_socket(socket_dgram_t* socket, sock_addr_inet_t* addr);
    static int32            dgram_net_receive(net_buf_t* buf, uint32 src_ip, uint16 src_port, uint32 dst_ip, uint16 dst_port);
    static void             get_not_used_port(socket_dgram_t* socket);

public:
    uint32                  m_ref;
    sock_addr_inet_t        m_addr;
    list_t<dgram_buf_t>     m_buffers;
    semaphore_t             m_buffer_sem;

    static spinlock_t       s_lock;
    static socket_dgram_t   s_dgram_sockets[MAX_DGRAM_SOCKET];
};

#endif
