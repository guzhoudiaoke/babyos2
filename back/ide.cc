/*
 * guzhoudiaoke@126.com
 * 2017-10-29
 */

#include "ide.h"
#include "babyos.h"
#include "x86.h"


ide_t::ide_t()
{
}
ide_t::~ide_t()
{
}

void ide_t::init()
{
    m_lock.init();
    m_cache.init();

    os()->get_arch()->get_8259a()->enable_irq(IRQ_HARDDISK);
    wait();
    outb(0x1f6, 0xe0 | (0 << 4));
    outb(0x1f6, 0xe0 | (1 << 4));
}

void ide_t::request(io_clb_t *clb)
{
    if (clb->read) {
        if (m_cache.read(clb)) {
            return;
        }
    }

    m_lock.lock();
    clb->next = NULL;
    if (m_head == NULL) {
        m_head = clb;
    }
    else {
        io_clb_t *p = m_head;
        while (p->next != NULL) {
            p = p->next;
        }
        p->next = clb;
    }
    m_lock.unlock();

    if (m_head == clb) {
        start(clb);
    }

    while ((clb->flags & IO_STATE_DONE) != IO_STATE_DONE) {
        current->sleep();
    }

    if (clb->read) {
        m_cache.insert(clb);
    }
    else {
        m_cache.write(clb);
    }
}

void ide_t::start(io_clb_t *clb)
{
    int lba = clb->lba;

    wait();

    outb(0x3f6, 0);     // generate interrupt
    outb(0x1f2, 1);     // sector num
    outb(0x1f3, lba & 0xff);
    outb(0x1f4, (lba >> 8)  & 0xff);
    outb(0x1f5, (lba >> 16) & 0xff);
    outb(0x1f6, 0xe0 | ((clb->dev & 0x1) << 4) | ((lba >> 24) & 0xff));

    if (clb->read) {
        outb(0x1f7, IO_CMD_READ);
    }
    else {
        outb(0x1f7, IO_CMD_WRITE);
        outsl(0x1f0, clb->buffer, SECT_SIZE/4);
    }
}

void ide_t::wait()
{
    while ((inb(0x1f7) & (HD_STATE_BUSY | HD_STATE_READY)) != HD_STATE_READY) {
        ;
    }
}

void ide_t::do_irq()
{
    m_lock.lock();

    if (m_head == NULL) {
        m_lock.unlock();
        return;
    }

    io_clb_t *clb = m_head;
    m_head = clb->next;
    m_lock.unlock();

    if (clb->read) {
        insl(0x1f0, clb->buffer, SECT_SIZE/4);
    }

    clb->flags |= IO_STATE_DONE;

    /* EOI */
    outb(0x20, 0x20);
    outb(0xa0, 0x20);

    if (m_head != NULL) {
        start(m_head);
    }
}

