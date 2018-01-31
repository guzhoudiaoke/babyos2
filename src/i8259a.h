/*
 * guzhoudiaoke@126.com
 * 2017-11-7
 */

#ifndef _I8259A_H_
#define _I8259A_H_

#include "types.h"

class i8259a_t {
public:
    i8259a_t();
    ~i8259a_t();

    void init();
    void enable_irq(uint32 irq);
};


#endif
