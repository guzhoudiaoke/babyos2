/*
 * guzhoudiaoke@126.com
 * 2018-03-01
 */

#include "rtl8139.h"
#include "pci.h"
#include "babyos.h"
#include "x86.h"
#include "string.h"

#define RTL8139_VENDOR_ID   0x10ec
#define RTL8139_DEVICE_ID   0x8139

#define RTL8139_MAC         0x00
#define RTL8139_TX_STATUS0  0x10
#define RTL8139_TX_ADDR0    0x20
#define RTL8139_RX_BUF_ADDR 0x30
#define RTL8139_COMMAND     0x37
#define RTL8139_INTR_MASK   0x3C
#define RTL8139_INTR_STATUS 0x3E
#define RTL8139_RCR         0x44
#define RTL8139_CONFIG_0    0x51
#define RTL8139_CONFIG_1    0x52

#define RTL8139_RX_OK       0x0001
#define RTL8139_RX_ERROR    0x0002
#define RTL8139_TX_OK       0x0004
#define RTL8139_TX_ERROR    0x0008
#define RTL8139_RX_OVERFLOW 0x0010
#define RTL8139_RX_UNDERRUN 0x0020 
#define RTL8139_RX_FIFOOVER 0x0040 
#define RTL8139_PCS_TIMEOUT 0x4000
#define RTL8139_PCI_ERROR   0x8000 

uint32 rtl8139_t::get_info_from_pci()
{
    /* get pci device */
    pci_device_t* device = os()->get_arch()->get_pci()->get_device(RTL8139_VENDOR_ID, RTL8139_DEVICE_ID);
    if (device == NULL) {
        console()->kprintf(RED, "RTL8139 init failed: not find pci device.\n");
        return -1;
    }

    os()->get_arch()->get_pci()->enable_bus_mastering(device);

    /* get io address */
    m_io_address = device->get_io_addr();
    if (m_io_address == 0) {
        console()->kprintf(RED, "RTL8139 init failed: INVALID pci device io address.\n");
        return -1;
    }

    /* get irq */
    m_irq = device->get_interrupt_line();
    if (m_irq == 0xff) {
        console()->kprintf(RED, "RTL8139 init failed: INVALID irq.\n");
        return -1;
    }

    return 0;
}

void rtl8139_t::init()
{
    console()->kprintf(CYAN, "*********************** init RTL8192 ****************************\n");

    if (get_info_from_pci() != 0) {
        return;
    }

    /* mac address */
    for (int i = 0; i < 6; i++) {
        m_mac_addr[i] = inb(m_io_address + RTL8139_MAC + i);
    }
    console()->kprintf(YELLOW, "mac addr: %2x:%2x:%2x:%2x:%2x:%2x\n", m_mac_addr[0], m_mac_addr[1],
            m_mac_addr[2], m_mac_addr[3], m_mac_addr[4], m_mac_addr[5]);

    /* send 0x00 to CONFIG_1 register to set the LWAKE + LWPTN to active high, 
     * this should essentially power on the device */
    outb(m_io_address + RTL8139_CONFIG_1, 0x00);

    /* software reset, to clear the RX and TX buffers and set everything back to defaults */
    outb(m_io_address + RTL8139_COMMAND, 0x10);
    while ((inb(m_io_address + RTL8139_COMMAND) & 0x10) != 0) {
        nop();
    }

    /* init receive buffer */
    m_rx_buffer = (uint8 *) os()->get_mm()->alloc_pages(0);
    uint32 rx_buffer_phy = VA2PA(m_rx_buffer);
    outl(m_io_address + RTL8139_RX_BUF_ADDR, rx_buffer_phy);

    /* 
     * set IMR(Interrupt Mask Register)
     * To set the RTL8139 to accept only the Transmit OK (TOK) and Receive OK (ROK) interrupts, 
     * we would have the TOK and ROK bits of the IMR high and leave the rest low. 
     * That way when a TOK or ROK IRQ happens, it actually will go through and fire up an IRQ.
     */
    outw(m_io_address + RTL8139_INTR_MASK, RTL8139_RX_OK | RTL8139_TX_OK);

    /* 
     * configuring receive buffer(RCR)
     * Before hoping to see a packet coming to you, you should tell the RTL8139 to accept packets 
     * based on various rules. The configuration register is RCR.
     *  AB - Accept Broadcast: Accept broadcast packets sent to mac ff:ff:ff:ff:ff:ff
     *  AM - Accept Multicast: Accept multicast packets.
     *  APM - Accept Physical Match: Accept packets send to NIC's MAC address.
     *  AAP - Accept All Packets. Accept all packets (run in promiscuous mode).
     *  Another bit, the WRAP bit, controls the handling of receive buffer wrap around.
     */
    outw(m_io_address + RTL8139_RCR, 0xf | (1 << 7)); /* AB + AM + APM + AAP */

    /* enable receive and transmitter */
    outb(m_io_address + RTL8139_COMMAND, 0x0c);

    /* enable interrupt */
    os()->get_arch()->get_io_apic()->enable_irq(m_irq, os()->get_arch()->get_cpu_num()-1);

    console()->kprintf(CYAN, "*********************** init RTL8192 ****************************\n");
}

uint32 rtl8139_t::get_irq()
{
    return m_irq;
}

void rtl8139_t::do_irq()
{
    console()->kprintf(PINK, "rtl8139 interrupt(by cpu %u): ", os()->get_arch()->get_current_cpu()->get_apic_id());

    uint32 status = inw(m_io_address + RTL8139_INTR_STATUS);
    outw(m_io_address + RTL8139_INTR_STATUS, status);

    if (status & RTL8139_RX_OK) {
        console()->kprintf(PINK, "RXOK\n");
    }
    if (status & RTL8139_TX_OK) {
        console()->kprintf(PINK, "TXOK\n");

        uint32 tx_status = inl(m_io_address + RTL8139_TX_STATUS0);
        console()->kprintf(YELLOW, "do_irq TX_STATUS0: 0x%8x\n", tx_status);
    }
    else {
        console()->kprintf(PINK, "0x%8x\n", status);
    }
}

int32 rtl8139_t::transmit(char* buf, uint32 len)
{
    static uint32 cur_tx = 0;
    static char* tx_buf[4];
    for (int i = 0; i < 4; i++) {
         tx_buf[i] = (char *) os()->get_mm()->alloc_pages(0);
    }

    memcpy(tx_buf[cur_tx], buf, len);

    uint32 status = inw(m_io_address + RTL8139_INTR_STATUS);
    console()->kprintf(GREEN, "transmit TX_STATUS0: 0x%8x, INTR_STATUS: %x\n", 
            inl(m_io_address + RTL8139_TX_STATUS0), status);

    uint32 flags;
    local_irq_save(flags);
    outl(m_io_address + RTL8139_TX_ADDR0 + cur_tx * 4, VA2PA(tx_buf[cur_tx]));
    outl(m_io_address + RTL8139_TX_STATUS0 + cur_tx * 4, (256 << 16) | 0x0 | len);
    cur_tx = (cur_tx + 1) % 4;
    console()->kprintf(GREEN, "after transmit TX_STATUS0: 0x%8x\n", inl(m_io_address + RTL8139_TX_STATUS0));
    restore_flags(flags);


    return 0;
}


//static char tx_buf[1536] = { 0xac, 0x22, 0x0b, 0x29, 0xdf, 0x8d, 0xec, 0x26,
//                             0xca, 0x80, 0xba, 0xb6, 0x08, 0x06, 0x00, 0x01,
//                             0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0xec, 0x26,
//                             0xca, 0x80, 0xb1, 0xb6, 0xc0, 0xa8, 0x01, 0x01,
//                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xa8,
//                             0x0a, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//                             0x00, 0x00, 0x00, 0x00};
