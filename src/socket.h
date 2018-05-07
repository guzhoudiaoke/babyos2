/*
 * guzhoudiaoke@126.com
 * 2018-01-20
 */

#ifndef _SOCKET_H_
#define _SOCKET_H_

#include "types.h"
#include "sem.h"
#include "sock_addr.h"
#include "sock_ring_buffer.h"

class socket_t {
public:
    typedef enum address_family_e {
        AF_LOCAL = 1,
        AF_INET = 2,
        AF_MAX,
    } address_family_t;

    typedef enum sock_type_e {
        SOCK_STREAM = 1,
        SOCK_DGRAM,
        SOCK_RAW,
    } sock_type_t;

    typedef enum protocol_e {
        PROTO_ICMP = 1,
        PROTO_TCP = 6,
        PROTO_UDP = 17,
    } protocol_t;

    typedef enum socket_state_e {
        SS_FREE = 0,
        SS_UNCONNECTED,
        SS_CONNECTING,
        SS_CONNECTED,
        SS_DISCONNECTING,
    } socket_state_t;

    typedef enum socket_flag_e {
        SF_ACCEPTCON = (1 << 16),   /* performed a listen */
        SF_WAITDATA  = (1 << 17),   /* wait data to read */
        SF_NOSPACE   = (1 << 18),   /* no space to write */
    } socket_flag_t;

    virtual int32 create(uint32 family, uint32 type, uint32 protocol);
    virtual int32 release() = 0;
    virtual int32 dup(socket_t* socket) = 0;
    virtual int32 get_addr(sock_addr_t* addr) = 0;

    virtual int32 bind(sock_addr_t* myaddr) = 0;
    virtual int32 listen(uint32 backlog) = 0;
    virtual int32 accept(socket_t* server_socket) = 0;
    virtual int32 connect(sock_addr_t* user_addr) = 0;

    virtual int32 read(void* buf, uint32 size) = 0;
    virtual int32 write(void* buf, uint32 size) = 0;
    virtual int32 send_to(void *buf, uint32 size, sock_addr_t* addr_to) = 0;
    virtual int32 recv_from(void *buf, uint32 size, sock_addr_t* addr_from) = 0;

public:
    uint32              m_flags;
    uint32              m_family;
    uint32              m_type;
    uint32              m_protocol;
    socket_state_t      m_state;
};

#endif

