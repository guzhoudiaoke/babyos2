/*
 * guzhoudiaoke@126.com
 * 2017-11-7
 */

#ifndef _I8259A_H_
#define _I8259A_H_

#include "types.h"

#define IRQ_0               (0x20)
#define IRQ_NUM             (0x10)
#define IRQ_TIMER           (0x0)
#define IRQ_KEYBOARD        (0x1)
#define IRQ_HARDDISK        (0xe)
#define IRQ_SYSCALL         (0x80)

class i8259a_t {
public:
    i8259a_t();
    ~i8259a_t();

    void init();
    void enable_irq(uint32 irq);
};


#endif
