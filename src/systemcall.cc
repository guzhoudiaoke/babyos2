/*
 * guzhoudiaoke@126.com
 * 2017-10-30
 */

#include "systemcall.h"
#include "babyos.h"

extern int32 sys_print(void);
static int32 (*system_call_table[])(void) = {
    sys_print,
};

int32 sys_print(void)
{
    console()->kprintf(GREEN, "sys_print()\n");
}

systemcall_t::systemcall_t()
{
}
systemcall_t::~systemcall_t()
{
}

void systemcall_t::do_syscall(trap_frame_t* frame)
{
    uint32 id = frame->eax;
    if (id >= MAX_SYSCALL) {
        console()->kprintf(RED, "unknown system call %u\n", id);
        frame->eax = -1;
    }
    else {
        frame->eax = system_call_table[id]();
    }
}
