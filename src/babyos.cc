/*
 * guzhoudiaoke@126.com
 * 2017-10-26
 */

#include "babyos.h"
#include "string.h"
#include "x86.h"
#include "vm.h"
#include "list.h"
#include "fs_test.h"
#include "sys_socket.h"
#include "delay.h"
#include "dns.h"

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

object_pool_t* babyos_t::get_obj_pool(uint32 type)
{
    if (type >= MAX_POOL) {
        return NULL;
    }
    return &m_pools[type];
}

object_pool_t*  babyos_t::get_obj_pool_of_size()
{
    return m_pool_of_size;
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

void babyos_t::start_init_proc()
{
    current->set_cwd(os()->get_fs()->get_root());
    
    int32 ret = 0;
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_FORK));

    if (ret == 0) {
        __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_EXEC), "b" ("/bin/init"), "c" (0));
    }
}

void babyos_t::init_pools()
{
    m_pools[VMA_POOL].init(sizeof(vm_area_t));
    m_pools[PIPE_POOL].init(sizeof(pipe_t));
    m_pools[TIMER_POOL].init(sizeof(timer_t));

    for (uint32 i = 1; i <= SMALL_POOL_SIZE; i++) {
        m_pool_of_size[i].init(i);
    }
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
    list.init(os()->get_obj_pool_of_size());

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
    //fs_tester_t tester;
    //tester.init(&m_fs);

    //m_console.kprintf(PINK, "**************** test fs ****************\n");
    //tester.dump_super_block();
    //tester.test_read_inode();
    //tester.test_ls("/");
    //tester.test_read_bitmap();
    //tester.test_read_dir_entry();
    //tester.test_namei();
    //tester.test_create();
    //tester.test_read();
    //tester.test_write();
    //tester.test_mkdir();
    //tester.test_link();
    //tester.test_unlink();
    //tester.test_ls("/bin");
    //m_console.kprintf(PINK, "**************** test fs ****************\n");
}

void test_rtl8139()
{
    static char str[64] = {0};
    static uint32 id = 0;
    static uint32 seq = 0;

    for (int i = 0; i < 100; i++) {
        //uint32 ip = net_t::make_ipaddr(192, 168, 1, 105);
        //if (os()->get_net()->get_ipaddr() != ip) {
        //    os()->get_net()->arp_request(ip);
        //}

        //uint32 ip2 = net_t::make_ipaddr(192, 168, 1, 103);
        //if (os()->get_net()->get_ipaddr() != ip2) {
        //    os()->get_net()->arp_request(ip2);
        //}
        delay_t::ms_delay(2000);

        uint32 ip = net_t::make_ipaddr(111, 13, 101, 208);
        if (os()->get_net()->get_ipaddr() != ip) {
            //os()->get_net()->arp_request(ip);
            //sprintf(str, "Hello 192.168.1.105, This is a raw ip package. id: %u", id++);
            //os()->get_net()->get_ip()->transmit(ip, (uint8 *) str, strlen(str) + 1, 0xff);
            os()->get_net()->get_icmp()->echo_request(ip, 0, seq++, NULL, 0);
        }
    }
}

void test_arp()
{
    static char str[64] = {0};
    static uint32 id = 0;
    static uint32 seq = 0;

    for (int i = 0; i < 100; i++) {
        uint32 ip = net_t::make_ipaddr(192, 168, 1, 105);
        if (os()->get_net()->get_ipaddr() != ip) {
            os()->get_net()->arp_request(ip);
        }
        delay_t::ms_delay(2000);
    }
}

void test_ip()
{
    static char str[64] = {0};
    static uint32 id = 0;

    for (int i = 0; i < 100; i++) {
        delay_t::ms_delay(2000);

        uint32 ip = net_t::make_ipaddr(111, 13, 101, 208);
        if (os()->get_net()->get_ipaddr() != ip) {
            os()->get_net()->arp_request(ip);
            sprintf(str, "Hello 192.168.1.105, This is a raw ip package. id: %u", id++);
            os()->get_net()->get_ip()->transmit(ip, (uint8 *) str, strlen(str) + 1, 0xff);
        }
    }
}

void test_icmp()
{
    static char str[64] = {0};
    static uint32 id = 0;
    static uint32 seq = 0;

    uint32 ip = net_t::make_ipaddr(111, 13, 101, 208);
    for (int i = 0; i < 100; i++) {
        delay_t::ms_delay(2000);

        if (os()->get_net()->get_ipaddr() != ip) {
            os()->get_net()->get_icmp()->echo_request(ip, 0, seq++, NULL, 0);
        }
    }
}

void test_dns()
{
    for (int i = 0; i < 1; i++) {
        dns_t::query("www.baidu.com");
        delay_t::ms_delay(2000);
    }
}

void test_tcp()
{
    static char str[64] = {0};
    strcpy(str, "Hello TCP");
    uint32 ip = net_t::make_ipaddr(192, 168, 1, 105);

    tcp_hdr_flag_t flag;
    flag.m_syn = 1;
    if (os()->get_net()->get_ipaddr() != ip) {
        os()->get_net()->get_tcp()->transmit(ip, 4096, 4097, 0, 0, flag, (uint8 *) str, strlen(str));
    }
}

void test_net()
{
    //test_rtl8139();
    //test_arp();
    //test_ip();
    //test_icmp();
    //test_dns();
    //test_tcp();
}

void babyos_t::run()
{
    atomic_set(&m_next_pid, 1);

    m_screen.init();
    m_console.init();

    m_console.kprintf(YELLOW, "Welcome to babyos\n");
    m_console.kprintf(YELLOW, "Author:\tguzhoudiaoke@126.com\n");

    m_mm.init();
    init_pools();

    m_arch.init();
    m_hd.init();
    m_block_dev.init(1);
    sys_socket_t::init();

    m_fs.init();
    test_fs();

    m_timer_mgr.init();
    m_process_mgr.init();
    m_net.init();

    /* start aps */
    m_arch.start_ap();

    /* start interrupt */
    sti();

    test_net();

    /* the first user process, init */
    start_init_proc();

    /* idle process */
    while (true) {
        while (os()->get_process_mgr()->get_run_queue()->empty()) {
            nop();
        }
        sti();
        m_arch.get_boot_processor()->schedule();
    }
}

void babyos_t::run_ap(uint32 index)
{
    cpu_t* cpu = os()->get_arch()->get_cpu(index);
    if (cpu == NULL) {
        panic("get an ERROR ap.");
    }

    /* set page dir */
    set_cr3(VA2PA(os()->get_mm()->get_kernel_pg_dir()));
    console()->kprintf(CYAN, "run ap of apic id: %x\n", local_apic_t::get_apic_id());

    /* start cpu */
    cpu->startup_ap();

    /* start interrupt */
    sti();

    /* schedule */
    while (true) {
        //while (current->m_need_resched == 0) {
        //    do_idle();
        //}
        while (os()->get_process_mgr()->get_run_queue()->empty()) {
            nop();
        }
        sti();
        cpu->schedule();
    }
}

void babyos_t::update(uint32 tick)
{
    /* arch */
    m_arch.update();

    /* timers */
    m_timer_mgr.update();

    /* console */
    m_console.update();
}

void babyos_t::panic(const char* s)
{
    cli();
    m_console.kprintf(RED, "[BABYOS PANICED], %s\n", s);
    while (1) {
        halt();
    }
}

file_system_t* babyos_t::get_fs()
{
    return &m_fs;
}

hard_disk_t* babyos_t::get_hd()
{
    return &m_hd;
}

block_dev_t* babyos_t::get_block_dev()
{
    return &m_block_dev;
}

timer_mgr_t* babyos_t::get_timer_mgr()
{
    return &m_timer_mgr;
}

process_mgr_t* babyos_t::get_process_mgr()
{
    return &m_process_mgr;
}

net_t* babyos_t::get_net()
{
    return &m_net;
}

