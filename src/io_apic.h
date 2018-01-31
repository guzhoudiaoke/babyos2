/*
 * guzhoudiaoke@126.com
 * 2018-01-29
 */

#ifndef _IO_APIC_H_
#define _IO_APIC_H_

#include "types.h"

#define IO_APIC_BASE     0xfec00000  /* default address map */

class io_apic_t {
public:
    void    init();
    void    enable_irq(uint32 irq, uint32 cpu_id);

private:
    uint32  read(uint32 reg);
    void    write(uint32 reg, uint32 data);

private:
};

#endif
