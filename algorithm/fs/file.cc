/*
 * guzhoudiaoke@126.com
 * 2018-01-02
 */

#include "file.h"
#include "fs.h"

extern file_system_t fs;

void file_t::init(uint32 type, inode_t* inode, uint32 offset, uint16 readable, uint16 writeable)
{
    m_type = type;
    m_ref = 1;
    m_readable = readable;
    m_writeable = writeable;
    m_inode = inode;
    m_offset = 0;
}

int file_t::close()
{
    if (m_ref < 1) {
        return -1;
    }

    if (--m_ref > 0) {
        return 0;
    }

    if (m_type == file_t::TYPE_INODE) {
        m_type = file_t::TYPE_NONE;
        fs.put_inode(m_inode);
    }
}

