/*
 * guzhoudiaoke@126.com
 * 2018-04-25
 */

#ifndef _SOCKET_STREAM_H_
#define _SOCKET_STREAM_H_

#include "types.h"
#include "socket.h"
#include "net_buf.h"
#include "sem.h"
#include "sock_ring_buffer.h"

#define MAX_STREAM_SOCKET    128

class tcp_hdr_t;
class socket_stream_t : public socket_t {
public:
    const uint32 c_max_buffer_num = 32;
    typedef enum tcp_state_e {
        ST_CLOSED,
        ST_LISTEN,
        ST_SYN_SENT,
        ST_SYN_RCVD,
        ST_ESTABLISHED,
        ST_FIN_WAIT_1,
        ST_FIN_WAIT_2,
        ST_CLOSE_WAIT,
        ST_CLOSING,
        ST_LAST_ACK,
        ST_TIME_WAIT,
    } tcp_state_t;

    socket_stream_t();
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
    int32 send_to(void* buf, uint32 size, sock_addr_t* addr_to);
    int32 recv_from(void* buf, uint32 size, sock_addr_t* addr_from);

    int32 net_receive(tcp_hdr_t* hdr, uint32 ip, uint16 port, net_buf_t* buf);


    static void             init_stream_sockets();
    static socket_t*        alloc_stream_socket();
    static void             release_stream_socket(socket_t* socket);
    static socket_stream_t* lookup_stream_socket(sock_addr_inet_t* addr);
    static int32            bind_stream_socket(socket_stream_t* socket, sock_addr_inet_t* addr);
    static int32            stream_net_receive(tcp_hdr_t* hdr, net_buf_t* buf, uint32 src_ip, uint32 dst_ip);
    static void             get_not_used_port(socket_stream_t* socket);

private:
    int32 tcp_syn(sock_addr_inet_t* server_addr);
    int32 tcp_ack(uint32 src_ip, uint16 src_port, uint32 dst_ip, uint16 dst_port, uint32 ack_no, bool syn);
    int32 tcp_send(uint32 src_ip, uint16 src_port, uint32 dst_ip, uint16 dst_port, uint8* data, uint32 len);
    int32 tcp_fin(uint32 src_ip, uint16 src_port, uint32 dst_ip, uint16 dst_port, uint32 ack_no);
    int32 net_receive_syn(tcp_hdr_t* hdr, uint32 src_ip, uint16 src_port);
    int32 net_receive_ack(tcp_hdr_t* hdr, uint32 src_ip, uint16 src_port);
    int32 net_receive_data(tcp_hdr_t* hdr, net_buf_t* buf);
    int32 net_receive_fin(tcp_hdr_t* hdr, uint32 src_ip, uint16 src_port);
    int32 tcp_close();

public:
    tcp_state_t              m_tcp_state;
    uint32                   m_ref;
    sock_addr_inet_t         m_addr;
    sock_ring_buffer_t       m_buffer;

    list_t<sock_addr_inet_t> m_connecting_list;
    sock_addr_inet_t         m_peer_addr;
    semaphore_t              m_sem;
    uint32                   m_seq_no;
    uint32                   m_ack_no;
    socket_stream_t*         m_dup_socket;

    static spinlock_t        s_lock;
    static socket_stream_t   s_stream_sockets[MAX_STREAM_SOCKET];
};

#endif
