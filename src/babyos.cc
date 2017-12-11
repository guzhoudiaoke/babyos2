/*
 * guzhoudiaoke@126.com
 * 2017-10-26
 */

#include "babyos.h"
#include "string.h"
#include "x86.h"
#include "vm.h"

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
        for (int i = 0; i < 100000000; i++);
        console()->kprintf(YELLOW, s);
    }
}

inline int32 fork()
{
    int32 ret = 0;
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (0x01));

    if (ret == 0) {
        console()->kprintf(WHITE, "int 0x80, return from child\n");
    }
    else {
        console()->kprintf(WHITE, "int 0x80, return from parent\n");
    }

    return ret;
}

inline void init()
{
    //delay_print("c,");

    int32 ret = 0;

    // fork
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (0x01));

    // child
    if (ret == 0) {
        delay_print("cc,");
    }

    // parent
    // exec
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (0x02));
    console()->kprintf(RED, "ERROR, should not reach here!!\n");
}

void test_syscall()
{
    int32 ret = 0;
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_FORK));

    if (ret == 0) {
        // exec
        __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_EXEC), "b" (512), "c" (32));
    }

    delay_print("P,");
}

void babyos_t::init_pools()
{
    m_pools[VMA_POOL].init(sizeof(vm_area_t));
}

void babyos_t::run()
{
    m_screen.init();
    m_console.init();

    m_console.kprintf(WHITE, "Welcome to babyos\n");
    m_console.kprintf(WHITE,   "Author:\tguzhoudiaoke@126.com\n");

    m_mm.init();
    m_arch.init();
    m_ide.init();
    init_pools();

    m_console.kprintf(WHITE, "sti()\n");
    sti();

    test_draw_time();
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

