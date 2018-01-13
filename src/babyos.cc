/*
 * guzhoudiaoke@126.com
 * 2017-10-26
 */

#include "babyos.h"
#include "string.h"
#include "x86.h"
#include "vm.h"
#include "list.h"

extern babyos_t babyos;
babyos_t* babyos_t::get_os()
{
    return &babyos;
}

babyos_t::babyos_t()
{
}

babyos_t::~babyos_t()
{
}

screen_t* babyos_t::get_screen() 
{
    return &m_screen; 
}

console_t* babyos_t::get_console()
{
    return &m_console;
}

mm_t* babyos_t::get_mm()
{
    return &m_mm;
}

arch_t* babyos_t::get_arch()
{
    return &m_arch;
}

ide_t* babyos_t::get_ide()
{
    return &m_ide;
}

object_pool_t* babyos_t::get_obj_pool(uint32 type)
{
    if (type >= MAX_POOL) {
        return NULL;
    }
    return &m_pools[type];
}

dev_op_t* babyos_t::get_dev(uint32 type)
{
    if (type >= MAX_DEV) {
        return NULL;
    }
    return &m_devices[type];
}

void test_draw_time()
{
    uint32 year, month, day, h, m, s;
    rtc_t *rtc = os()->get_arch()->get_rtc();
    year = rtc->year();
    month = rtc->month();
    day = rtc->day();
    h = rtc->hour();
    m = rtc->minute();
    s = rtc->second();
    console()->kprintf(GREEN, "%d-%d-%d %d:%d:%d\n", year, month, day, h, m, s);
}

inline void delay_print(char* s)
{
    while (1) {
        for (int i = 0; i < 300000000; i++);
        console()->kprintf(YELLOW, s);
    }
}

void test_syscall()
{
    int32 ret = 0;

    // fork
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_FORK));

    if (ret == 0) {
        // exec
        //__asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_EXEC), "b" (1024), "c" (32), "d" ("init"));
        __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_EXEC), "b" ("/bin/init"));
    }

    //delay_print("P,");

    while (true) {
        os()->get_arch()->get_cpu()->schedule();
    }
}

void babyos_t::init_pools()
{
    m_pools[VMA_POOL].init(sizeof(vm_area_t));
}

void print_list(list_t<int>& list) {
    list_t<int>::iterator it = list.begin();
    while (it != list.end()) {
        console()->kprintf(WHITE, "%u, ", *it);
        it++;
    }
    console()->kprintf(WHITE, "\n");
}

int data[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
void test_list()
{
    list_t<int> list;
    list.init();

    console()->kprintf(WHITE, "test list...\n");
    if (list.empty()) {
        console()->kprintf(WHITE, "list is empty.\n");
    }
    else {
        console()->kprintf(WHITE, "list is not empty.\n");
    }

    for (int i = 2; i < 8; i++) {
        list.push_back(data[i]);
    }

    print_list(list);

    list_t<int>::iterator it = list.begin();
    list.insert(it, data[0]);
    print_list(list);

    it = list.end();
    list.insert(it, data[1]);
    print_list(list);

    it = list.begin();
    it++;
    it++;
    list.insert(it, data[9]);
    print_list(list);

    it = list.begin();
    list.erase(it);
    print_list(list);

    it = list.begin();
    for (int i = 0; i < 7; i++) {
        it++;
    }
    list.erase(it);
    print_list(list);

    it = list.begin();
    it++;
    it++;
    it++;
    list.erase(it);
    print_list(list);

    list.push_front(123);
    list.push_front(234);
    list.push_back(345);
    list.push_back(456);
    print_list(list);

    list.pop_front();
    print_list(list);
    list.pop_front();
    print_list(list);

    list.pop_back();
    print_list(list);
}

void babyos_t::test_fs()
{
    //m_console.kprintf(PINK, "**************** test fs ****************\n");
    //m_fs.dump_super_block();
    //m_fs.test_read_inode();
    //m_fs.test_ls("/");
    //m_fs.test_read_bitmap();
    //m_fs.test_read_dir_entry();
    //m_fs.test_namei();
    //m_fs.test_create();
    //m_fs.test_read();
    //m_fs.test_write();
    //m_fs.test_mkdir();
    //m_fs.test_link();
    //m_fs.test_unlink();
    //m_fs.test_ls("/bin");
    //m_console.kprintf(PINK, "**************** test fs ****************\n");
}

void babyos_t::run()
{
    atomic_set(&m_next_pid, 0);

    m_screen.init();
    m_console.init();

    m_console.kprintf(YELLOW, "Welcome to babyos\n");
    m_console.kprintf(YELLOW,   "Author:\tguzhoudiaoke@126.com\n");

    m_mm.init();
    m_arch.init();
    m_ide.init();
    init_pools();

    // test list
    //test_list();

    //m_console.kprintf(WHITE, "sti()\n");
    sti();

    m_fs.init();
    m_arch.get_cpu()->get_idle()->set_cwd(m_fs.get_root());

    test_fs();

    //test_draw_time();
    test_syscall();

    while (1) {
        halt();
    }
}

void babyos_t::update(uint32 tick)
{
    // arch
    m_arch.update();

    // console
    m_console.update();
}

uint32 babyos_t::get_next_pid()
{
    uint32 pid = atomic_read(&m_next_pid);
    atomic_inc(&m_next_pid);
    return pid;
}

file_system_t* babyos_t::get_fs()
{
    return &m_fs;
}

