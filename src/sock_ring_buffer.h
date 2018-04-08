/*
 * guzhoudiaoke@126.com
 * 2018-04-07
 * separate from socket.h/c
 */

#ifndef _SOCK_BUFFER_H_
#define _SOCK_BUFFER_H_

#include "types.h"
#include "spinlock.h"
#include "sem.h"

#define SOCK_BUF_SIZE       4096

class socket_t;
class sock_ring_buffer_t {
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

#endif
