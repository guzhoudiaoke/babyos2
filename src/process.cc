/*
 * guzhoudiaoke@126.com
 * 2017-12-27
 */

#include "process.h"
#include "x86.h"
#include "babyos.h"
#include "string.h"
#include "elf.h"

extern void ret_from_isr(void) __asm__("ret_from_isr");

process_t* process_t::fork(trap_frame_t* frame)
{
    // alloc a process_t
    process_t* p = (process_t *)os()->get_mm()->alloc_pages(1);
    *p = *this;

    // frame
    trap_frame_t* child_frame = ((trap_frame_t *) ((uint32(p) + PAGE_SIZE*2))) - 1;
    memcpy(child_frame, frame, sizeof(trap_frame_t));
    child_frame->eax = 0;

    // vmm
    p->m_vmm.copy(m_vmm);

    // signal
    p->m_signal.copy(m_signal);
    m_sig_queue.init();
    m_sig_mask_lock.init();

    // context
    p->m_context.esp = (uint32) child_frame;
    p->m_context.esp0 = (uint32) (child_frame + 1);
    p->m_context.eip = (uint32) ret_from_isr;

    // pid, need check if same with other process
    p->m_pid = os()->get_next_pid();

    // change state
    p->m_state = PROCESS_ST_RUNABLE;
    p->m_need_resched = 0;
    p->m_timeslice = 10;
    p->m_children.init();
    p->m_parent = this;

    return p;
}

static int32 init_user_stack(trap_frame_t* frame)
{
    vm_area_t* vma = (vm_area_t *) os()->get_obj_pool(VMA_POOL)->alloc_from_pool();
    if (vma == NULL) {
        console()->kprintf(RED, "BUG on alloc vm_area_t!\n");
        return -1;
    }
    vma->m_end   = USER_STACK_TOP;
    vma->m_start = USER_STACK_TOP - PAGE_SIZE;
    vma->m_page_prot = PROT_READ | PROT_WRITE;
    vma->m_flags = VM_STACK;
    vma->m_next = NULL;
    if (current->m_vmm.insert_vma(vma) != 0) {
        console()->kprintf(RED, "BUG on insert vma!\n");
        return -1;
    }

    frame->esp = USER_STACK_TOP - 4;
}

int32 process_t::exec(trap_frame_t* frame)
{
    // copy process name
    const char* name = (const char *) frame->edx;
    strcpy(m_name, name);

    // flush old mmap
    current->m_vmm.release();

    // load elf binary
    if (elf_t::load(frame) != 0) {
        return -1;
    }

    // code segment, data segment and so on
    frame->cs = (SEG_UCODE << 3 | 0x3);
    frame->ds = frame->es = frame->ss = frame->fs = frame->gs = (SEG_UDATA << 3 | 0x3);

    // stack, esp
    init_user_stack(frame);

    return 0;
}

static void process_timeout(uint32 data)
{
    os()->get_arch()->get_cpu()->wake_up_process((process_t *) data);
}

void process_t::sleep(uint32 ticks)
{
    cli();

    // add a timer to alarm after ticks
    timer_t timer;
    timer.init(ticks, (uint32) current, process_timeout);
    os()->get_arch()->get_cpu()->add_timer(&timer);

    current->m_state = PROCESS_ST_SLEEP;
    sti();

    os()->get_arch()->get_cpu()->schedule();

    // remove the timer
    os()->get_arch()->get_cpu()->remove_timer(&timer);
}

void process_t::sleep()
{
    // FIXME: only test
    __asm__("nop");
}

void process_t::wake_up()
{
    m_state = PROCESS_ST_RUNABLE;
}

void process_t::adope_children()
{
    process_t* child_reaper = os()->get_arch()->get_cpu()->get_child_reaper();
    list_t<process_t *>::iterator it = m_children.begin();
    while (it != m_children.end()) {
        (*it)->m_parent = child_reaper;
        it++;
    }
}

void process_t::notify_parent()
{
    // SIGCHLD is needed, but now not support signal
    // so just wake up parent
    os()->get_arch()->get_cpu()->wake_up_process(m_parent);
}

int32 process_t::wait_children(int32 pid)
{
repeat:
    cli();
    m_state = PROCESS_ST_SLEEP;
    list_t<process_t *>::iterator it = m_children.begin();

    bool flag = false;
    for (it; it != m_children.end(); it++) {
        process_t* p = *it;
        if (pid != -1 && pid != p->m_pid) {
            continue;
        }

        // find pid, or any child if pid == -1
        flag = true;

        if (p->m_state != PROCESS_ST_ZOMBIE) {
            continue;
        }

        // this child has be ZOMBIE, free it
        os()->get_arch()->get_cpu()->release_process(p);
        goto end_wait;
    }
    sti();

    // if find pid, or any child if pid == -1
    if (flag) {
        // sleep to wait child exit
        os()->get_arch()->get_cpu()->schedule();

        // wake up by a exited child, repead to check
        goto repeat;
    }

end_wait:
    // continue to run
    m_state = PROCESS_ST_RUNABLE;
    return 0;
}

int32 process_t::exit()
{
    console()->kprintf(YELLOW, "\ncurrent: %p(%s), pid: %x is exiting\n", current, current->m_name, current->m_pid);

    // remove the resource, now only memory 
    m_vmm.release();

    // adope children to init
    adope_children();

    // do not schedule before finish to notify parent
    cli();

    m_state = PROCESS_ST_ZOMBIE;

    // let parent wake up, and mourn us
    notify_parent();

    sti();

    os()->get_arch()->get_cpu()->schedule();

    return 0;
}

void process_t::calc_sig_pending()
{
    m_sig_pending = !m_sig_queue.empty();
}

