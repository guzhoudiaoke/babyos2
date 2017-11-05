/*
 * guzhoudiaoke@126.com
 * 2017-10-30
 */

#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include "types.h"

//#define SYS_PRINT 0
//#define MAX_SYSCALL 1

enum {
    SYS_PRINT = 0,
    SYS_FORK,
    SYS_EXEC,
    SYS_SCHED,
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
