/*
 * guzhoudiaoke@126.com
 * 2017-12-27
 */

#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "types.h"
#include "list.h"
#include "vm.h"

/* get current by kernel stack */
class process_t;
static inline process_t* get_current(void)
{
    process_t* current;
    __asm__("andl %%esp,%0; ":"=r" (current) : "0" (~8191UL));

    return current;
}
#define current get_current()


/* state of process */
enum {
    PROCESS_ST_PREPARE = 0,
    PROCESS_ST_RUNABLE,
    PROCESS_ST_RUNNING,
    PROCESS_ST_SLEEP,
    PROCESS_ST_ZOMBIE,
};

typedef struct context_s {
    uint32	esp0;
    uint32	eip;
    uint32	esp;
    uint32	fs;
    uint32	gs;
} context_t;

class process_t {
public:
    process_t* fork(trap_frame_t* frame);
    int32 exec(trap_frame_t* frame);
    void sleep(uint32 ticks);
    void sleep();
    void wake_up();
    int32 exit();
    int32 wait_children(int32 pid);

private:
    void notify_parent();
    void adope_children();

public:
    uint32              m_need_resched;
    char		        m_name[32];
    pid_t		        m_pid;
    uint32		        m_state;
    context_t	        m_context;
    vmm_t		        m_vmm;
    uint32              m_timeslice;

    process_t*          m_parent;
    list_t<process_t *> m_children;

    process_t*	        m_prev;
    process_t*	        m_next;
};

#endif
