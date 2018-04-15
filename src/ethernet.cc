/*
 * guzhoudiaoke@126.com
 * 2018-03-18
 */

#include "ethernet.h"
#include "string.h"
#include "net.h"
#include "babyos.h"

void ethernet_hdr_t::init(uint8 dst[ETH_ADDR_LEN], uint8 source[ETH_ADDR_LEN], uint16 proto)
{
    memcpy(m_dest, dst, ETH_ADDR_LEN);
    memcpy(m_source, source, ETH_ADDR_LEN);
    m_proto = proto;
}

/***************************************************************************/

void ethernet_t::set_eth_addr(uint8 eth_addr[ETH_ADDR_LEN])
{
    memcpy(m_eth_addr, eth_addr, ETH_ADDR_LEN);
}

void ethernet_t::transmit(uint8 eth_addr[ETH_ADDR_LEN], uint16 protocol, uint8* data, uint32 len)
{
    uint32 length = len;
    if (length < 46) {
        length = 46;
    }

    if (memcmp(eth_addr, m_eth_addr, ETH_ADDR_LEN) == 0) {
        return;
    }

    uint32 total = length + sizeof(ethernet_hdr_t);
    net_buf_t* buf = os()->get_net()->alloc_net_buffer(total);
    if (buf != NULL) {
        ethernet_hdr_t eth_hdr;
        eth_hdr.init(eth_addr, m_eth_addr, net_t::ntohs(protocol));

        buf->append(&eth_hdr, sizeof(ethernet_hdr_t));
        buf->append(data, len);
        if (length > len) {
            buf->append_zero(length-len);
        }

        /* transmit */
        os()->get_arch()->get_rtl8139()->transmit(buf);

        /* free the buffer */
        os()->get_net()->free_net_buffer(buf);
    }
    else {
        console()->kprintf(RED, "Allocate net buffer failed.\n");
    }
}

void ethernet_t::receive(uint8* data, uint32 len)
{
    net_buf_t* buf = os()->get_net()->alloc_net_buffer(len);
    if (buf != NULL) {
        buf->append(data, len);
        ethernet_hdr_t* hdr = (ethernet_hdr_t *) buf->get_data();

        if (net_t::htons(hdr->m_proto) == PROTO_ARP) {
            buf->pop_front(sizeof(ethernet_hdr_t));
            os()->get_net()->get_arp()->receive(hdr->m_source, buf);
            os()->get_net()->free_net_buffer(buf);
        }
        else if (net_t::htons(hdr->m_proto) == PROTO_IP) {
            buf->pop_front(sizeof(ethernet_hdr_t));
            os()->get_net()->get_ip()->receive(buf);
        }
        else {
            //console()->kprintf(YELLOW, "net receive from [%2x:%2x:%2x:%2x:%2x:%2x] to [%2x:%2x:%2x:%2x:%2x:%2x]\n",
            //        hdr->m_source[0], hdr->m_source[1], hdr->m_source[2], hdr->m_source[3], hdr->m_source[4], hdr->m_source[5],
            //        hdr->m_dest[0], hdr->m_dest[1], hdr->m_dest[2], hdr->m_dest[3], hdr->m_dest[4], hdr->m_dest[5]);
            if (memcmp(m_eth_addr, hdr->m_dest, ETH_ADDR_LEN) == 0) {
                console()->kprintf(GREEN, "data: %s\n", (char *) buf->get_data() + sizeof(ethernet_hdr_t));
            }

            os()->get_net()->free_net_buffer(buf);
        }
    }
}

uint32 ethernet_t::ether_crc(uint8* data, int32 length)
{
    static const uint32 crc_table[] = {
        0x4DBDF21C, 0x500AE278, 0x76D3D2D4, 0x6B64C2B0,
        0x3B61B38C, 0x26D6A3E8, 0x000F9344, 0x1DB88320,
        0xA005713C, 0xBDB26158, 0x9B6B51F4, 0x86DC4190,
        0xD6D930AC, 0xCB6E20C8, 0xEDB71064, 0xF0000000
    };

    uint32 crc = 0;
    for (int n = 0; n < length; n++) {
        crc = (crc >> 4) ^ crc_table[(crc ^ (data[n] >> 0)) & 0x0F];  /* lower nibble */
        crc = (crc >> 4) ^ crc_table[(crc ^ (data[n] >> 4)) & 0x0F];  /* upper nibble */
    }

    return crc;
}

uint8* ethernet_t::get_addr()
{
    return m_eth_addr;
}

