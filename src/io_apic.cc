/*
 * guzhoudiaoke@126.com
 * 2018-01-29
 */

#include "io_apic.h"
#include "traps.h"
#include "babyos.h"

#define IO_APIC_REG_ID      0x00
#define IO_APIC_REG_VER     0x01
#define IO_APIC_REG_TABLE   0x10    /* redirection table */

#define INT_DISABLED        0x00010000

void io_apic_t::init()
{
    uint32 count = (read(IO_APIC_REG_VER) >> 16) & 0xff;
    uint32 id = (read(IO_APIC_REG_ID) >> 24);

    //console()->kprintf(GREEN, "****************** io apic ****************\n", id);
    //console()->kprintf(GREEN, "io apic id: %u\n", id);
    //console()->kprintf(GREEN, "io apic num of table: %u\n", count);
    //console()->kprintf(GREEN, "****************** io apic ****************\n", id);

    for (int i = 0; i <= count; i++) {
        write(IO_APIC_REG_TABLE + 2*i,     INT_DISABLED | (IRQ_0 + i));
        write(IO_APIC_REG_TABLE + 2*i + 1, 0);
    }
}

void io_apic_t::enable_irq(uint32 irq, uint32 cpu_id)
{
    write(IO_APIC_REG_TABLE + 2*irq,     IRQ_0 + irq);
    write(IO_APIC_REG_TABLE + 2*irq + 1, cpu_id << 24);
}

uint32 io_apic_t::read(uint32 reg)
{
    uint32* base = (uint32 *) IO_APIC_BASE;
    *base = reg;
    return *(base + 4);
}

void io_apic_t::write(uint32 reg, uint32 data)
{
    uint32* base = (uint32 *) IO_APIC_BASE;
    *base = reg;
    *(base + 4) = data;
}

