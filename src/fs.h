/*
 * guzhoudiaoke@126.com
 * 2018-01-05
 */

#ifndef _FS_H_
#define _FS_H_

#include "types.h"
#include "file.h"
#include "file_table.h"
#include "inode.h"
#include "sock_addr.h"

#define ROOT_DEV 1
#define ROOT_INUM 1 

#define BSIZE   SECT_SIZE

#define MAX_PATH 14
#define MAX_INODE_CACHE 64

typedef struct super_block_s {
    uint32 m_size;      /* total num of blocks */
    uint32 m_nblocks;   /* num of data blocks */
    uint32 m_ninodes;   /* num of inodes */
} super_block_t;

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


class fs_tester_t;
class file_system_t {
    friend class fs_tester_t;

public:
    void     init();
    inode_t* get_root();
    uint32   inode_block(uint32 id);
    uint32   inode_offset(uint32 id);

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
    int      do_pipe(int fd[2]);
    int      do_send_to(int fd, void* buffer, uint32 count, sock_addr_t* addr);
    int      do_recv_from(int fd, void* buffer, uint32 count, sock_addr_t* addr);

    inode_t* dup_inode(inode_t* inode);
    void     put_inode(inode_t* inode);

    file_t*  dup_file(file_t* file);
    file_t*  alloc_file();
    int      close_file(file_t* file);

private:
    uint32   bitmap_block();

    void     read_super_block(super_block_t* sb);
    void     zero_block(uint32 dev, uint32 b);
    uint32   alloc_block(uint32 dev);
    void     free_block(uint32 dev, uint32 b);
    uint32   block_map(inode_t* inode, uint32 block);

    inode_t* alloc_inode(uint16 dev, uint16 type);
    inode_t* get_inode(uint32 dev, uint32 inum);
    int      read_inode(inode_t* inode, void* dst, uint32 offset, uint32 size);
    int      write_inode(inode_t* inode, void* src, uint32 offset, uint32 size);

    int      dir_link(inode_t* inode, char* name, uint32 inum);
    inode_t* dir_lookup(inode_t* inode, char* name, unsigned& offset);
    bool     dir_empty(inode_t* inode);

    inode_t* namei(const char* path);
    inode_t* nameiparent(const char* path, char *name);
    inode_t* namei(const char* path, int parent, char* name);
    inode_t* create(const char* path, uint16 type, uint16 major, uint16 minor);

    int      alloc_pipe(file_t*& file_read, file_t*& file_write);

private:
    uint32          m_dev;
    uint32          m_super_block_lba;
    uint32          m_inode_lba;

    super_block_t   m_super_block;
    inode_t*        m_root;

    spinlock_t      m_inodes_lock;
    inode_t         m_inodes[MAX_INODE_CACHE];

    file_table_t    m_file_table;
};


#endif
