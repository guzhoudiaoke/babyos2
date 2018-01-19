/*
 * guzhoudiaoke@126.com
 * 2018-01-05
 */

#ifndef _FS_H_
#define _FS_H_

#include "types.h"
#include "file.h"
#include "sem.h"

#define ROOT_DEV 1
#define ROOT_INUM 1 

#define BSIZE   SECT_SIZE
#define NDIRECT 12
#define NINDIRECT (BSIZE / sizeof(unsigned int))
#define MAX_FILE_SIZE (NDIRECT + NINDIRECT)

#define MAX_PATH 14
#define MAX_INODE_CACHE 64
#define MAX_FILE_NUM 64

typedef struct super_block_s {
    uint32 m_size;      /* total num of blocks */
    uint32 m_nblocks;   /* num of data blocks */
    uint32 m_ninodes;   /* num of inodes */
} super_block_t;

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

typedef struct dir_entry_s {
public:
    uint16 m_inum;
    char   m_name[MAX_PATH];
} dir_entry_t;

typedef struct stat_s {
    uint16 m_type;
    uint16 m_nlinks;
    uint32 m_dev;
    uint32 m_size;
} stat_t;


class file_system_t {
public:
    void     init();
    inode_t* get_root();

    int      do_open(const char* path, int mode);
    int      do_close(int fd);
    int      do_read(int fd, void* buffer, uint32 count);
    int      do_write(int fd, void* buffer, uint32 count);
    int      do_mkdir(const char* path);
    int      do_link(const char* path_old, const char* path_new);
    int      do_unlink(const char* path);
    int      do_mknod(const char* path, int major, int minor);
    int      do_dup(int fd);
    int      do_seek(int fd, uint32 pos);
    int      do_stat(int fd, stat_t* st);
    int      do_chdir(const char* path);

    inode_t* dup_inode(inode_t* inode);
    void     put_inode(inode_t* inode);
    file_t*  dup_file(file_t* file);
    int      close_file(file_t* file);

    /* test */
    void     dump_super_block();
    void     test_read_inode();
    void     test_read_bitmap();
    void     test_read_dir_entry();
    void     test_namei();
    void     test_create();
    void     test_read();
    void     test_write();
    void     test_mkdir();
    void     test_link();
    void     test_unlink();
    void     test_ls(const char* path);

private:
    uint32   inode_block(uint32 id);
    uint32   inode_offset(uint32 id);
    uint32   bitmap_block();

    void     read_super_block();
    void     zero_block(uint32 dev, uint32 b);
    uint32   alloc_block(uint32 dev);
    void     free_block(uint32 dev, uint32 b);
    uint32   block_map(inode_t* inode, uint32 block);

    void     read_disk_inode(int id, inode_t* inode);
    void     update_disk_inode(inode_t* inode);
    inode_t* get_inode(uint32 dev, uint32 inum);
    int      read_inode(inode_t* inode, void* dst, uint32 offset, uint32 size);
    int      write_inode(inode_t* inode, void* src, uint32 offset, uint32 size);
    inode_t* alloc_inode(uint16 dev, uint16 type);

    int      dir_link(inode_t* inode, char* name, uint32 inum);
    inode_t* dir_lookup(inode_t* inode, char* name, unsigned& offset);
    bool     dir_empty(inode_t* inode);

    inode_t* namei(const char* path);
    inode_t* nameiparent(const char* path, char *name);
    inode_t* namei(const char* path, int parent, char* name);
    inode_t* create(const char* path, uint16 type, uint16 major, uint16 minor);

    file_t*  alloc_file();

private:
    uint32          m_dev;
    uint32          m_super_block_lba;
    uint32          m_inode_lba;

    super_block_t   m_super_block;
    inode_t*        m_root;

    spinlock_t      m_inodes_lock;
    inode_t         m_inodes[MAX_INODE_CACHE];

    spinlock_t      m_file_table_lock;
    file_t          m_file_table[MAX_FILE_NUM];
};


#endif
