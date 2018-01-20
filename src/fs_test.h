/*
 * split from class fs
 * guzhoudiaoke@126.com
 * 2018-01-20
 */

#ifndef _FS_TEST_H_
#define _FS_TEST_H_

#include "types.h"
#include "fs.h"

class fs_tester_t {
public:
    void init(file_system_t* fs);

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
    file_system_t* m_fs;
};

#endif
