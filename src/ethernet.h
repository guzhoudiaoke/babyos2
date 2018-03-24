/*
 * guzhoudiaoke@126.com
 * 2018-03-18
 */

#ifndef _ETHERNET_H_
#define _ETHERNET_H_

#include "types.h"

#define ETH_ADDR_LEN        6

typedef struct ethernet_hdr_s {
    uint8   m_dest[ETH_ADDR_LEN];
    uint8   m_source[ETH_ADDR_LEN];
    uint16  m_proto;

    void init(uint8 dst[ETH_ADDR_LEN], uint8 source[ETH_ADDR_LEN], uint16 proto);
} ethernet_hdr_t;

class ethernet_t {
public:
    void init();
    void set_eth_addr(uint8 mac_addr[ETH_ADDR_LEN]);
    void transmit(uint8 eth_addr[ETH_ADDR_LEN], uint16 protocol, uint8* data, uint32 len);
    void receive(uint8* data, uint32 len);
    uint32 ether_crc(uint8* data, int32 length);
    uint8* get_addr();

private:
    uint8 m_eth_addr[ETH_ADDR_LEN];
};

#endif
