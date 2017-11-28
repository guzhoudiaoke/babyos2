/*
 * 2017-11-27
 * guzhoudiaoke@126.com
 */

#include "babyos.h"
#include "mm.h"
#include "pool.h"


void object_pool_t::init(uint32 obj_size)
{
	m_obj_size = obj_size;
	m_available = 0;
	m_free_list = NULL;
}

void object_pool_t::free_object(void* obj)
{
	object_pool_obj_t* o = (object_pool_obj_t*) obj;
	o->m_next = NULL;
	if (m_free_list == NULL) {
		m_free_list = o;
	}
	else {
		o->m_next = m_free_list;
		m_free_list = o;
	}
	m_available++;
}

void* object_pool_t::alloc_from_pool()
{
	if (m_free_list == NULL) {
		uint8* mem = (uint8 *) os()->get_mm()->alloc_pages(0);
		uint8* end = mem + PAGE_SIZE;
		while (mem + m_obj_size <= end) {
			free_object(mem);
			mem += m_obj_size;
		}
	}

	void* obj = m_free_list;
	m_free_list = m_free_list->m_next;

	m_available--;
	return obj;
}

uint32 object_pool_t::get_available()
{
	return m_available;
}

