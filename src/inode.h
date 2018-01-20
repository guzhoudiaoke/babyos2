/*
 * split from fs.h/fs.cc
 * guzhoudiaoke@126.com
 * 2018-01-20
 */

#ifndef _INODE_H_
#define _INODE_H_

#include "types.h"
#include "sem.h"

#define NDIRECT 12
#define NINDIRECT (BSIZE / sizeof(unsigned int))
#define MAX_FILE_SIZE (NDIRECT + NINDIRECT)

class disk_inode_t {
public:

public:
    uint16 m_type;
    uint16 m_major;
    uint16 m_minor;
    uint16 m_nlinks;
    uint32 m_size;
    uint32 m_addrs[NDIRECT + 1];
};


class inode_t {
public:
    enum inode_type {
        I_TYPE_DIR = 1,
        I_TYPE_FILE,
        I_TYPE_DEV,
    };
    void init(uint16 major, uint16 minor, uint16 nlink);
    void lock();
    void unlock();
    void read_from_disk(int id);
    void write_to_disk();

public:
    uint32 m_dev;           // device number
    uint32 m_inum;          // inode number
    uint32 m_ref;           // reference count
    uint32 m_valid;
    semaphore_t m_sem;

    uint16 m_type;          // copy of disk inode
    uint16 m_major;
    uint16 m_minor;
    uint16 m_nlinks;
    uint32 m_size;
    uint32 m_addrs[NDIRECT+1];
};


#endif
