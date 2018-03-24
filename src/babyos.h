/*
 * guzhoudiaoke@126.com
 * 2017-10-26
 */

#ifndef _BABYOS_H_
#define _BABYOS_H_

#include "screen.h"
#include "console.h"
#include "mm.h"
#include "arch.h"
#include "pool.h"
#include "atomic.h"
#include "fs.h"
#include "file.h"
#include "block_dev.h"
#include "hd.h"
#include "pipe.h"
#include "timer_mgr.h"
#include "process_mgr.h"
#include "net.h"

enum pool_type_e {
	VMA_POOL = 0,
    PIPE_POOL,
    TIMER_POOL,
	MAX_POOL,
};

enum device_type_e {
    DEV_CONSOLE = 0,
    MAX_DEV,
};

class babyos_t {
public:
    babyos_t();
    ~babyos_t();

    void run();
    void run_ap(uint32 index);
	void update(uint32 tick);
    void panic(const char* s);
    void start_init_proc();

    screen_t*       get_screen();
    console_t*      get_console();
    mm_t*           get_mm();
    arch_t*         get_arch();
	object_pool_t*  get_obj_pool(uint32 type);
	object_pool_t*  get_obj_pool_of_size();
    file_system_t*  get_fs();
    dev_op_t*       get_dev(uint32 type);
    hard_disk_t*    get_hd();
    block_dev_t*    get_block_dev();
    timer_mgr_t*    get_timer_mgr();
    process_mgr_t*  get_process_mgr();
    net_t*          get_net();

    static babyos_t* get_os();

private:
	void init_pools();
    void test_fs();

private:
    screen_t		m_screen;
    console_t		m_console;
    mm_t			m_mm;
    arch_t			m_arch;
	object_pool_t	m_pools[MAX_POOL];
	object_pool_t	m_pool_of_size[SMALL_POOL_SIZE+1];
	atomic_t	    m_next_pid;
    file_system_t   m_fs;
    dev_op_t        m_devices[MAX_DEV];
    hard_disk_t     m_hd;
    block_dev_t     m_block_dev;
    timer_mgr_t     m_timer_mgr;
    process_mgr_t   m_process_mgr;
    net_t           m_net;
};

#define os()	    babyos_t::get_os()
#define console()   os()->get_console()

#endif

