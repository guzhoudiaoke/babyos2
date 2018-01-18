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


enum pool_type_e {
	VMA_POOL = 0,
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
	void update(uint32 tick);

    screen_t*   get_screen();
    console_t*  get_console();
    mm_t*       get_mm();
    arch_t*     get_arch();
	object_pool_t* get_obj_pool(uint32 type);
    object_pool_t* get_obj_pool_of_size(uint32 size);
    uint32      get_next_pid();
    file_system_t* get_fs();
    dev_op_t*   get_dev(uint32 type);
    hard_disk_t* get_hd();
    block_dev_t* get_block_dev();

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
	atomic_t	    m_next_pid;
    file_system_t   m_fs;
    dev_op_t        m_devices[MAX_DEV];
    hard_disk_t     m_hd;
    block_dev_t     m_block_dev;
};

#define os()	    babyos_t::get_os()
#define console()   os()->get_console()

#endif
