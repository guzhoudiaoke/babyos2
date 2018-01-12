/*
 * guzhoudiaoke@126.com
 * 2018-01-02
 */

#ifndef _FS_H_
#define _FS_H_

#include "bitmap.h"

#define BSIZE   512
#define NDIRECT 12
#define NINDIRECT (BSIZE / sizeof(unsigned int))
#define MAXFILE (NDIRECT + NINDIRECT)

#define MAX_PATH 14
#define MAX_INODE_CACHE 64
#define MAX_FILE 10

#define ROOT_DEV 1
#define ROOT_INUM 1 

#define I_BUSY 0x1
#define I_VALID 0x2

typedef unsigned short uint16;
typedef unsigned int uint32;


class file_t;
class inode_t;
class process_t {
public:
    void init();
    int  alloc_fd(file_t* file);
public:
    file_t *m_files[MAX_FILE];
    inode_t *m_cwd;
};



class super_block_t {
public:
    uint32 m_size;
    uint32 m_nblocks;
    uint32 m_ninodes;
};


class inode_t {
public:
    enum inode_type {
        I_TYPE_DIR = 1,
        I_TYPE_FILE,
    };

    void init(uint16 major, uint16 minor, uint16 nlink);
    void lock();
    void unlock();

public:
    uint32 m_dev;           // Device number
    uint32 m_inum;          // Inode number
    uint32 m_ref;            // Reference count
    uint32 m_flags;

    uint16 m_type;         // copy of disk inode
    uint16 m_major;
    uint16 m_minor;
    uint16 m_nlinks;
    uint32 m_size;
    uint32 m_addrs[NDIRECT+1];
};

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

class dir_entry_t {
public:
    uint16 m_inum;
    char   m_name[MAX_PATH];
};

class file_system_t {
public:
    void     init();

    int      do_open(char* path, int mode);
    int      do_close(int fd);
    int      do_read(int fd, void* buffer, unsigned count);
    int      do_write(int fd, void* buffer, unsigned count);
    int      do_mkdir(char* path);
    int      do_link(char* path_old, char* path_new);
    int      do_unlink(char* path);

    void     read_block(unsigned block_index, unsigned char* buffer);
    void     write_block(unsigned block_index, unsigned char* buffer);
    void     zero_block(uint32 dev, uint32 b);
    unsigned alloc_block(unsigned dev);
    void     free_block(unsigned dev, unsigned b);

    file_t*  alloc_file();
    inode_t* create(char* path, uint16 type, uint16 major, uint16 minor);
    inode_t* get_inode(uint32 dev, uint32 inum);
    inode_t* dup_inode(inode_t* inode);
    inode_t* put_inode(inode_t* inode);
    inode_t* namei(char* path);
    inode_t* nameiparent(char* path, char *name);
    inode_t* alloc_inode(uint16 dev, uint16 type);
    inode_t* dir_lookup(inode_t* inode, char* name, unsigned& offset);
    int      dir_link(inode_t* inode, char* name, uint32 inum);
    void     update_disk_inode(inode_t* inode);
    void     read_disk_inode(unsigned index, inode_t* dinode);
    int      read_inode(inode_t* inode, char* dst, uint32 offset, uint32 size);
    int      write_inode(inode_t* inode, char* src, uint32 offset, uint32 size);

private:
    void     read_super_block();
    inode_t* namei(char* path, int parent, char* name);
    uint32   block_map(inode_t* inode, uint32 block);
    bool     dir_empty(inode_t* inode);

public:
    super_block_t   m_super_block;
    inode_t         m_inodes[MAX_INODE_CACHE];
    bitmap_t        m_block_bmp;
};

#endif

