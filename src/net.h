/*
 * guzhoudiaoke@126.com
 * 2018-03-10
 */

#ifndef _NET_H_
#define _NET_H_

#include "types.h"
#include "list.h"
#include "spinlock.h"
#include "ethernet.h"
#include "arp.h"
#include "ip.h"

#define NET_BUF_SIZE        256
#define NET_BUF_DATA_SIZE   232
#define NET_BUF_PAGES       1024

#define EXT_BUF_SIZE        2048
#define EXT_BUF_PAGES       1024

/* protocols */
#define PROTO_IP            0x0800
#define PROTO_ARP           0x0806


__inline uint16 htons(uint16 n)
{
  return ((n & 0xFF) << 8) | ((n & 0xFF00) >> 8);
}

__inline uint16 ntohs(uint16 n)
{
  return ((n & 0xFF) << 8) | ((n & 0xFF00) >> 8);
}

__inline uint32 htonl(uint32 n)
{
  return ((n & 0xFF) << 24) | ((n & 0xFF00) << 8) | ((n & 0xFF0000) >> 8) | ((n & 0xFF000000) >> 24);
}

__inline uint32 ntohl(uint32 n)
{
  return ((n & 0xFF) << 24) | ((n & 0xFF00) << 8) | ((n & 0xFF0000) >> 8) | ((n & 0xFF000000) >> 24);
}

typedef struct packet_s {
    uint32              m_packet_len;
    list_t<net_buf_t *> m_buffer_list;
} packet_t;

class net_t {
public:
    void init();
    void set_eth_addr(uint8 mac_addr[ETH_ADDR_LEN]);
    void transmit(uint8 eth_addr[ETH_ADDR_LEN], uint8* data, uint32 len);
    void receive(uint8* data, uint32 len);
    void arp_request(uint32 ip);

    net_buf_t* alloc_net_buffer(uint32 len);
    void free_net_buffer(net_buf_t* buf);

    ethernet_t* get_ethernet();
    arp_t* get_arp();
    ip_t* get_ip();

    uint32 get_ipaddr();
    uint32 get_subnet_mask();
    uint32 get_gateway();

    static uint32 make_ipaddr(uint8 ip0, uint8 ip1, uint8 ip2, uint8 ip3);
    static uint16 check_sum(uint8* data, uint32 len);
    static void dump_ip_addr(uint32 ip);
    static void dump_eth_addr(uint8 eth_addr[ETH_ADDR_LEN]);

private:
    list_t<net_buf_t *> m_net_buffers;
    spinlock_t          m_buffer_lock;
    ethernet_t          m_ethernet;
    arp_t               m_arp;
    ip_t                m_ip;

    uint32              m_ipaddr;
    uint32              m_subnet_mask;
    uint32              m_gateway;
};

#endif
