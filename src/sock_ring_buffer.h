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

class socket_t;
class sock_ring_buffer_t {
public:
    void   init(socket_t* socket, uint32 order);
    int32  get_char(uint8& ch);
    int32  put_char(uint8 ch);
    uint32 get_free_space();

    int32  get_data(uint8* data, uint32 max);
    int32  put_data(uint8* data, uint32 len);

private:
    socket_t*   m_socket;
    uint8*      m_buffer;
    uint32      m_length;
    uint32      m_left;
    uint32      m_read_index;
    uint32      m_write_index;
    spinlock_t  m_lock;
    semaphore_t m_wait_space;
    semaphore_t m_wait_item;
};

#endif
