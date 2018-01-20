/*
 * split from fs
 * guzhoudiaoke@126.com
 * 2018-01-20
 */

#ifndef _FILE_TABLE_H_
#define _FILE_TABLE_H_

#include "types.h"
#include "file.h"
#include "spinlock.h"

#define MAX_FILE_NUM 256


class file_table_t {
public:
    void    init();
    file_t* alloc();
    int     free(file_t* file);
    file_t* dup_file(file_t* file);

private:
    spinlock_t      m_lock;
    file_t          m_file_table[MAX_FILE_NUM];
};

#endif
