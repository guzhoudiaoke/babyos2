/*
 * guzhoudiaoke@126.com
 * 2017-10-27
 */

#include "queue.h"

queue_t::queue_t()
{
}

queue_t::~queue_t()
{
}

void queue_t::init()
{
	m_front = 0;
	m_rear = 0;
}

int32 queue_t::empty()
{
	if (m_rear == m_front)
		return 1;
	
	return 0;
}

int32 queue_t::full()
{
	if ((m_rear + 1) % BUFFER_SIZE == m_front)
		return 1;
	
	return 0;
}

int32 queue_t::en_queue(uint8 val)
{
	if (full())
		return -1;

	m_buffer[m_rear] = val;
	m_rear = (m_rear + 1) % BUFFER_SIZE;

	return 0;
}

int32 queue_t::de_queue(uint8 *val)
{
	if (empty())
		return -1;

	*val = m_buffer[m_front];
	m_front = (m_front + 1) % BUFFER_SIZE;
	
	return 1;
}

