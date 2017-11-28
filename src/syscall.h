/*
 * guzhoudiaoke@126.com
 * 2017-10-30
 */

#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include "types.h"

enum {
    SYS_PRINT = 0,
    SYS_FORK,
    SYS_EXEC,
	SYS_MMAP,
    MAX_SYSCALL,
};


class syscall_t {
public:
    syscall_t();
    ~syscall_t();

    void do_syscall(trap_frame_t* frame);

private:
};

#endif
