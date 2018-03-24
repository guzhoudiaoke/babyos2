/*
 * guzhoudiaoke@126.com
 * 2018-01-29
 * split from timer.h
 */

#include "i8254.h"
#include "babyos.h"
#include "x86.h"


i8254_t::i8254_t()
{
}
i8254_t::~i8254_t()
{
}

void i8254_t::init()
{
    m_tick = 0;

    uint32 val = CLOCK_TICK_RATE / HZ;

    /* control */
    outb(0x43, 0x34);

    /* clock 0 */
    outb(0x40, uint8(val & 0xff));
    outb(0x40, uint8(val >> 8));

	os()->get_arch()->get_8259a()->enable_irq(IRQ_TIMER);		/* enable keyboard interrupt */
}

void i8254_t::do_irq()
{
    m_tick++;
    os()->update(m_tick);

    /* EOI */
    outb(0x20, 0x20);
}

uint32 i8254_t::get_timer_count()
{
    uint32 count = 0;
    outb(0x43, 0x00);
    count = inb(0x40);
    count |= inb(0x40) << 8;

    return count;
}

//uint64 i8254_t::get_tick()
//{
//    return m_tick;
//}

