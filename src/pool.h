/*
 * 2017-11-27
 * guzhoudiaoke@126.com
 */

#ifndef _POOL_H_
#define _POOL_H_

#include "types.h"
#include "spinlock.h"

#define SMALL_POOL_SIZE 64

typedef struct object_pool_obj_s {
	struct object_pool_obj_s*	m_next;
} object_pool_obj_t;

class object_pool_t {
public:
	void init(uint32 obj_size);
	void free_object_nolock(void* obj);
	void free_object(void* obj);
	void* alloc_from_pool();
	uint32 get_available();

private:
	uint32				m_obj_size;
	uint32				m_available;
	object_pool_obj_t*	m_free_list;
    spinlock_t          m_lock;
};

#endif
