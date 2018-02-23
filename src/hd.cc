/*
 * guzhoudiaoke@126.com
 * 2018-01-17
 */

#include "hd.h"
#include "babyos.h"
#include "x86.h"

void request_t::init(uint32 dev, uint32 lba, uint32 cmd, io_buffer_t* b)
{
    m_dev = dev;
    m_lba = lba;
    m_cmd = cmd;
    m_buffer = b;
}

/***********************************************************/

void hard_disk_t::init()
{
    m_lock.init();
    m_req_list.init(os()->get_obj_pool_of_size());
    m_current = NULL;

    //os()->get_arch()->get_8259a()->enable_irq(IRQ_HARDDISK);
    os()->get_arch()->get_io_apic()->enable_irq(IRQ_HARDDISK, 0);
    wait();
    outb(0x1f6, 0xe0 | (1 << 4));
}

void hard_disk_t::add_request(request_t* req)
{
    if (m_current == NULL) {
        m_current = req;
        do_request();
    }
    else {
        uint32 flags;
        m_lock.lock_irqsave(flags);
        m_req_list.push_back(req);
        m_lock.unlock_irqrestore(flags);
    }
}

void hard_disk_t::do_request()
{
    if (m_current == NULL) {
        return;
    }

    uint32 lba = m_current->m_lba;
    wait();

    outb(0x3f6, 0);     // generate interrupt
    outb(0x1f2, 1);     // sector num
    outb(0x1f3, lba & 0xff);
    outb(0x1f4, (lba >> 8)  & 0xff);
    outb(0x1f5, (lba >> 16) & 0xff);
    outb(0x1f6, 0xe0 | ((m_current->m_dev & 0x1) << 4) | ((lba >> 24) & 0xff));

    if (m_current->m_cmd == request_t::CMD_READ) {
        outb(0x1f7, IO_CMD_READ);
    }
    else {
        outb(0x1f7, IO_CMD_WRITE);
        outsl(0x1f0, m_current->m_buffer->m_buffer, SECT_SIZE/4);
    }
}

void hard_disk_t::end_request()
{
    if (m_current->m_cmd == request_t::CMD_READ) {
        insl(0x1f0, m_current->m_buffer->m_buffer, SECT_SIZE/4);
    }

    m_current->m_buffer->m_done = 1;
    m_current->m_buffer->done();

    m_current = NULL;
    if (!m_req_list.empty()) {
        m_current = *m_req_list.begin();
        m_req_list.pop_front();
    }

    /* EOI */
    //outb(0x20, 0x20);
    //outb(0xa0, 0x20);
    os()->get_arch()->get_current_cpu()->get_local_apic()->eoi();

    do_request();
}

void hard_disk_t::wait()
{
    while ((inb(0x1f7) & (HD_STATE_BUSY | HD_STATE_READY)) != HD_STATE_READY) {
        ;
    }
}

void hard_disk_t::do_irq()
{
    end_request();
}


