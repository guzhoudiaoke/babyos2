/*
 * guzhoudiaoke@126.com
 * 2018-01-06
 */

#include "file.h"

void file_t::init(uint32 type, inode_t* inode, pipe_t* pipe, uint32 offset, uint16 readable, uint16 writeable)
{
    m_type = type;
    m_ref = 1;
    m_readable = readable;
    m_writeable = writeable;
    m_inode = inode;
    m_pipe = pipe;
    m_offset = 0;
    m_socket = NULL;
}

void file_t::init(uint32 type, socket_t* socket)
{
    m_type = TYPE_SOCKET;
    m_ref = 1;
    m_readable = 1;
    m_writeable = 1;
    m_inode = NULL;
    m_pipe = NULL;
    m_offset = 0;
    m_socket = socket;
}
