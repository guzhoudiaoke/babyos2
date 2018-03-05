/*
 * guzhoudiaoke@126.com
 * 2018-03-01
 */

#ifndef _RTL_8139_H_
#define _RTL_8139_H_

#include "types.h"

#define NUM_TX_BUFFER   4

class rtl8139_t {
public:
    void init();
    uint32 get_irq();
    void do_irq();
    int32 transmit(char* buf, uint32 len);

private:
    uint32 get_info_from_pci();

private:
    uint32  m_io_address;
    uint32  m_irq;
    uint8*  m_rx_buffer;
    uint8*  m_tx_buffers[NUM_TX_BUFFER];
    uint8   m_mac_addr[6];
};

#endif
