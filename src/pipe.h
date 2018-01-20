/*
 * guzhoudiaoke@126.com
 * 2018-01-20
 */

#ifndef _PIPE_H_
#define _PIPE_H_

#include "types.h"
#include "sem.h"
#include "spinlock.h"

#define PIPE_BUF_SIZE 512

class pipe_t {
public:
    void  init();
    int   get_char(char& ch);
    int   put_char(char ch);

    int32 read(void* buf, uint32 size);
    int32 write(void* buf, uint32 size);
    void  close(bool write_end);

private:
    char        m_buffer[PIPE_BUF_SIZE];
    uint32      m_read_index;
    uint32      m_write_index;
    spinlock_t  m_lock;
    semaphore_t m_space;    // how many space can use to put
    semaphore_t m_item;     // how many item can get

    bool        m_readable;
    bool        m_writable;
};

#endif
