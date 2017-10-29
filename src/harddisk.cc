/*
 * guzhoudiaoke@126.com
 * 2017-10-29
 */

#include "harddisk.h"
#include "babyos.h"
#include "x86.h"

Harddisk::Harddisk()
{
}
Harddisk::~Harddisk()
{
}

void Harddisk::init()
{
    os()->get_arch()->get_8259a()->enable_irq(IRQ_HARDDISK);
    wait();
    outb(0x1f6, 0xe0 | (0 << 4));
}

void Harddisk::request(IO_clb_t *clb)
{
    m_lock.lock();

    clb->m_next = NULL;
    if (m_head == NULL) {
        m_head = clb;
    }
    else {
        IO_clb_t *p = m_head;
        while (p->m_next != NULL) {
            p = p->m_next;
        }
        p->m_next = clb;
    }
    m_lock.unlock();

    if (m_head == clb) {
        start(clb);
    }

    while ((clb->m_flags & IO_STATE_DONE) != IO_STATE_DONE) {
        os()->get_arch()->get_cpu()->sleep();
    }

}

void Harddisk::start(IO_clb_t *clb)
{
    int lba = clb->m_lba;

    wait();

    outb(0x3f6, 0);     // generate interrupt
    outb(0x1f2, 1);     // sector num
    outb(0x1f3, lba & 0xff);
    outb(0x1f4, (lba >> 8)  & 0xff);
    outb(0x1f5, (lba >> 16) & 0xff);
    outb(0x1f6, 0xe0 | ((clb->m_dev & 0x1) << 4) | ((lba >> 24) & 0xff));

    if (clb->m_read) {
        outb(0x1f7, IO_CMD_READ);
    }
    else {
        outb(0x1f7, IO_CMD_WRITE);
        outsl(0x1f0, clb->m_buffer, SECT_SIZE/4);
    }
}

void Harddisk::wait()
{
    while ((inb(0x1f7) & (HD_STATE_BUSY | HD_STATE_READY)) != HD_STATE_READY) {
        ;
    }
}

void Harddisk::do_irq()
{
    os()->get_console()->kprintf(WHITE, "Harddisk::do_irq()\n");

    m_lock.lock();

    if (m_head == NULL) {
        m_lock.unlock();
        return;
    }

    IO_clb_t *clb = m_head;
    m_head = clb->m_next;
    m_lock.unlock();

    insl(0x1f0, clb->m_buffer, SECT_SIZE/4);

    clb->m_flags |= IO_STATE_DONE;
    // wakeup()

    if (m_head != NULL) {
        start(m_head);
    }

}

