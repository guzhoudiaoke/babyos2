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
#include "ide.h"
#include "pool.h"
#include "atomic.h"

enum pool_type_e {
	VMA_POOL = 0,
	MAX_POOL,
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
    ide_t*      get_ide();
	object_pool_t* get_obj_pool(uint32 type);
    uint32      get_next_pid();

    static babyos_t* get_os();

private:
	void init_pools();

private:
    screen_t		m_screen;
    console_t		m_console;
    mm_t			m_mm;
    arch_t			m_arch;
    ide_t			m_ide;
	object_pool_t	m_pools[MAX_POOL];
	atomic_t	    m_next_pid;
};

#define os()	    babyos_t::get_os()
#define console()   os()->get_console()

#endif
