/*
 * guzhoudiaoke@126.com
 * 2018-03-01
 */

#ifndef _RTL_8139_H_
#define _RTL_8139_H_

#include "types.h"
#include "net.h"

#define NUM_TX_BUFFER   4

class rtl8139_t {
public:
    void init();
    uint32 get_irq();
    void do_irq();
    int32 transmit(net_buf_t* buf);

private:
    uint32 get_info_from_pci();
    void   receive();

private:
    bool    m_inited;
    uint32  m_io_address;
    uint32  m_irq;
    uint8   m_mac_addr[6];

    uint8*  m_rx_buffer;
    uint32  m_current_rx;   /* CAPR, Current Address of Packet Read */
    uint32  m_rx_buf_len;

    uint8*  m_tx_buffers[NUM_TX_BUFFER];
    uint8   m_current_tx;
};

#endif
