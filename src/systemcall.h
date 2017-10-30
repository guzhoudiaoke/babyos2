/*
 * guzhoudiaoke@126.com
 * 2017-10-30
 */

#ifndef _SYSTEMCALL_H_
#define _SYSTEMCALL_H_

#include "types.h"

#define SYS_PRINT 0
#define MAX_SYSCALL 1


class systemcall_t {
public:
    systemcall_t();
    ~systemcall_t();

    void do_syscall(trap_frame_t* frame);

private:
};

#endif
