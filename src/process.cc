/*
 * guzhoudiaoke@126.com
 * 2017-12-27
 */

#include "process.h"
#include "x86.h"
#include "babyos.h"
#include "string.h"
#include "elf.h"

extern void ret_from_fork(void) __asm__("ret_from_fork");

process_t* process_t::fork(trap_frame_t* frame)
{
    // alloc a process_t
    process_t* p = (process_t *)os()->get_mm()->alloc_pages(1);
    if (p == NULL) {
        console()->kprintf(RED, "fork failed\n");
        return NULL;
    }

    *p = *this;

    // frame
    trap_frame_t* child_frame = ((trap_frame_t *) ((uint32(p) + PAGE_SIZE*2))) - 1;
    memcpy(child_frame, frame, sizeof(trap_frame_t));
    child_frame->eax = 0;

    // vmm
    p->m_vmm.copy(m_vmm);

    // signal
    p->m_signal.copy(m_signal);
    m_sig_queue.init(os()->get_obj_pool_of_size());
    m_sig_mask_lock.init();

    // file
    p->m_cwd = os()->get_fs()->dup_inode(m_cwd);
    for (int i = 0; i < MAX_OPEN_FILE; i++) {
        if (m_files[i] != NULL && m_files[i]->m_type != file_t::TYPE_NONE) {
            p->m_files[i] = os()->get_fs()->dup_file(m_files[i]);
        }
    }

    /* context */
    p->m_context.esp = (uint32) child_frame;
    p->m_context.esp0 = (uint32) (child_frame + 1);
    p->m_context.eip = (uint32) ret_from_fork;

    /* pid, need check if same with other process */
    p->m_pid = os()->get_process_mgr()->get_next_pid();

    /* change state */
    p->m_state = PROCESS_ST_RUNNING;
    p->m_need_resched = 0;
    p->m_timeslice = 2;

    p->m_children.init(os()->get_obj_pool_of_size());
    p->m_wait_child.init();
    p->m_parent = this;
    p->m_has_cpu = 0;


    /* link to run queue */
    os()->get_process_mgr()->add_process_to_rq(p);

    /* add to process list */
    os()->get_process_mgr()->add_process_to_list(p);

    /* add a child for current */
    m_children.push_back(p);

    return p;
}

static int32 init_user_stack(trap_frame_t* frame, argument_t* arg)
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
    if (arg == NULL) {
        return 0;
    }

    // space for args
    for (int i = 0; i < arg->m_argc; i++) {
        frame->esp -= (strlen(arg->m_argv[i]) + 1);
    }
    frame->esp -= frame->esp % 4;
    frame->esp -= arg->m_argc * sizeof(char*);  // argv[]
    frame->esp -= arg->m_argc * sizeof(char**); // argv
    frame->esp -= sizeof(uint32);               // argc
    frame->esp -= sizeof(uint32);               // ret address

    // ret addr
    uint32 top = frame->esp;
    *(uint32 *)top = 0xffffffff;
    top += sizeof(uint32);

    // argc
    *(uint32 *)top = arg->m_argc;
    top += sizeof(uint32);

    // argv
    *(uint32 *)top = top + sizeof(char **);
    top += sizeof(uint32);

    char** argv = (char **) top;
    char* p = (char *) top + sizeof(char *) * arg->m_argc;
    for (int i = 0; i < arg->m_argc; i++) {
        argv[i] = p;
        strcpy(p, arg->m_argv[i]);
        p += (strlen(p) + 1);
    }

    return 0;
}

int32 process_t::exec(trap_frame_t* frame)
{
    // copy process name
    const char* path = (const char *) frame->ebx;
    strcpy(m_name, path);

    //console()->kprintf(PURPLE, "X%u\n", current->m_pid);

    // save arg
    argument_t* arg = NULL;
    if (frame->ecx != 0) {
        arg = (argument_t *) os()->get_mm()->alloc_pages(0);
        memcpy(arg, (void *)frame->ecx, sizeof(argument_t));
    }

    // flush old mmap
    current->m_vmm.release();

    // load elf binary
    if (elf_t::load(frame, m_name) != 0) {
        exit();
        return -1;
    }

    // code segment, data segment and so on
    frame->cs = (SEG_UCODE << 3 | 0x3);
    frame->ds = frame->es = frame->ss = frame->fs = frame->gs = (SEG_UDATA << 3 | 0x3);

    // stack, esp
    init_user_stack(frame, arg);

    // free arg
    if (arg != NULL) {
        os()->get_mm()->free_pages(arg, 0);
    }

    return 0;
}

static void process_timeout(uint32 data)
{
    process_t* proc = (process_t *) data;
    os()->get_process_mgr()->wake_up_process(proc);
}

void process_t::sleep(uint32 ticks)
{
    // add a timer to alarm after ticks
    timer_t timer;
    timer.init(ticks, (uint32) current, process_timeout);
    os()->get_timer_mgr()->add_timer(&timer);

    current->m_state = PROCESS_ST_SLEEP;

    os()->get_arch()->get_current_cpu()->schedule();

    // remove the timer
    os()->get_timer_mgr()->remove_timer(&timer);
}

void process_t::sleep_on(wait_queue_t* queue)
{
    queue->add(current);
    //console()->kprintf(RED, "S%u\t", current->m_pid);
    current->m_state = PROCESS_ST_SLEEP;
    os()->get_arch()->get_current_cpu()->schedule();
    queue->remove(current);
}

void process_t::set_state(uint32 state)
{
    m_state = state;
}

void process_t::adope_children()
{
    process_t* child_reaper = os()->get_process_mgr()->get_child_reaper();
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
    m_parent->m_wait_child.wake_up();
}

int32 process_t::wait_children(int32 pid)
{
    current->m_wait_child.add(current);

repeat:
    m_state = PROCESS_ST_SLEEP;
    list_t<process_t *>::iterator it = m_children.begin();

    bool flag = false;
    for (it; it != m_children.end(); it++) {
        process_t* p = *it;
        if (pid != -1 && pid != p->m_pid) {
            continue;
        }

        /* find pid, or find any child if pid == -1 */
        flag = true;

        if (p->m_state != PROCESS_ST_ZOMBIE) {
            continue;
        }

        /* this child has become ZOMBIE, free it */
        os()->get_process_mgr()->release_process(p);

        goto end_wait;
    }

    /* if find pid, or any child if pid == -1 */
    if (flag) {
        /* sleep to wait child exit */
        os()->get_arch()->get_current_cpu()->schedule();

        /* wake up by a exited child, repead to check */
        goto repeat;
    }

end_wait:
    /* continue to run */
    m_state = PROCESS_ST_RUNNING;
    current->m_wait_child.remove(current);

    return 0;
}

int32 process_t::exit()
{
    //console()->kprintf(BLACK, "E%u\t", current->m_pid);
    /* remove the mem resource */
    m_vmm.release();

    /* close all opend files */
    os()->get_fs()->put_inode(m_cwd);
    for (int i = 0; i < MAX_OPEN_FILE; i++) {
        if (m_files[i] != NULL && m_files[i]->m_type != file_t::TYPE_NONE) {
            os()->get_fs()->close_file(m_files[i]);
        }
    }

    /* adope children to init */
    adope_children();

    /* set state as sleep */
    m_state = PROCESS_ST_ZOMBIE;

    /* let parent wake up, and mourn us */
    notify_parent();

    /* schedule other process */
    os()->get_arch()->get_current_cpu()->schedule();

    return 0;
}

void process_t::calc_sig_pending()
{
    m_sig_pending = !m_sig_queue.empty();
}

int process_t::alloc_fd(file_t* file)
{
    for (int i = 0; i < MAX_OPEN_FILE; i++) {
        if (m_files[i] == NULL) {
            m_files[i] = file;
            return i;
        }
    }
    return -1;
}

file_t* process_t::get_file(int fd)
{
    if (fd < 0 || fd >= MAX_OPEN_FILE) {
        return NULL;
    }
    return m_files[fd];
}

void process_t::free_fd(int fd)
{
    if (fd >= 0 && fd < MAX_OPEN_FILE) {
        current->m_files[fd] = NULL;
    }
}

void process_t::set_cwd(inode_t* inode)
{
    m_cwd = inode;
}

extern "C"
void do_signal(trap_frame_t* frame)
{
    current->do_signal(frame);
}

void process_t::do_signal(trap_frame_t* frame)
{
    if (!m_sig_queue.empty()) {
        siginfo_t si = *current->m_sig_queue.begin();
        m_sig_queue.pop_front();
        calc_sig_pending();
        m_signal.handle_signal(frame, si);
    }
}

void process_t::lock()
{
    m_task_lock.lock();
}

void process_t::unlock()
{
    m_task_lock.unlock();
}

