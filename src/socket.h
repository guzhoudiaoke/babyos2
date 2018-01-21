/*
 * guzhoudiaoke@126.com
 * 2018-01-20
 */

#ifndef _SOCKET_H_
#define _SOCKET_H_

#include "types.h"
#include "spinlock.h"
#include "sem.h"

#define SOCK_BUF_SIZE       4096

class sock_addr_t {
public:
    uint16 m_family;        /* address family, AF_xxx */
};


class socket_t;
class sock_buffer_t {
public:
    void init(socket_t* socket);
    int  get_char(char& ch);
    int  put_char(char ch);

private:
    socket_t*   m_socket;
    char        m_buffer[SOCK_BUF_SIZE];
    uint32      m_read_index;
    uint32      m_write_index;
    spinlock_t  m_lock;
    semaphore_t m_wait_space;
    semaphore_t m_wait_item;
};


class socket_t {
public:
    typedef enum address_family_e {
        AF_LOCAL,
        AF_MAX,
    } address_family_t;

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
    virtual int32 get_name(sock_addr_t* addr) = 0;

    virtual int32 bind(sock_addr_t* myaddr) = 0;
    virtual int32 listen(uint32 backlog) = 0;
    virtual int32 accept(socket_t* server_socket) = 0;
    virtual int32 connect(sock_addr_t* user_addr) = 0;
    virtual int32 read(void* buf, uint32 size) = 0;
    virtual int32 write(void* buf, uint32 size) = 0;

public:
    uint32              m_flags;
    uint32              m_family;
    uint32              m_type;
    uint32              m_protocol;
    socket_state_t      m_state;

    list_t<socket_t *>  m_connecting_list;
    socket_t*           m_connected_socket;

    semaphore_t         m_wait_connect_sem;
    semaphore_t         m_wait_accept_sem;
};

#endif

