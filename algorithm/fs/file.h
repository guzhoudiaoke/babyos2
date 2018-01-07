/*
 * guzhoudiaoke@126.com
 * 2018-01-02
 */

#ifndef _FILE_H_
#define _FILE_H_

#include "fs.h"

class file_t {
public:
    enum {
        MODE_RDONLY = 0x1,
        MODE_WRONLY = 0x2,
        MODE_RDWR = 0x4,
        MODE_CREATE = 0x200,
    };
    enum {
        TYPE_NONE,
        TYPE_PIPE,
        TYPE_INODE,
    };
    enum {
        PROT_READ = 1,
        PROT_WRITE = 2,
    };

    void init(uint32 type, inode_t* inode, uint32 offset, uint16 readable, uint16 writeable);
    int  read(void* buffer, unsigned count);
    int  close();

public:
    uint32      m_type;
    uint32      m_ref;
    uint16      m_readable;
    uint16      m_writeable;
    inode_t*    m_inode;
    uint32      m_offset;
};

#endif
