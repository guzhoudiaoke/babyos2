/*
 * guzhoudiaoke@126.com
 * 2017-10-27
 */

#ifndef _QUEUE_H_
#define _QUEUE_H_

#include "types.h"

#define BUFFER_SIZE		0x100		/* buffer size */

class queue_t {
public:
    queue_t();
    ~queue_t();

    void init();
    int32 empty();
    int32 full();
    int32 en_queue(uint8 val);
    int32 de_queue(uint8 *val);

private:
	uint32 m_front;
	uint32 m_rear;
	uint8  m_buffer[BUFFER_SIZE];
};

#endif
