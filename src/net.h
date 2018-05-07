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
#include "icmp.h"
#include "net_buf.h"
#include "udp.h"
#include "tcp.h"

#define NET_BUF_PAGES       1024
#define EXT_BUF_PAGES       1024

/* protocols */
#define PROTO_IP            0x0800
#define PROTO_ARP           0x0806


class net_t {
public:
    void init();
    void set_eth_addr(uint8 mac_addr[ETH_ADDR_LEN]);
    void transmit(uint8 eth_addr[ETH_ADDR_LEN], uint8* data, uint32 len);
    void receive(uint8* data, uint32 len);
    void arp_request(uint32 ip);

    net_buf_t* alloc_net_buffer(uint32 len);
    void free_net_buffer(net_buf_t* buf);
    uint8* alloc_ext_buffer();
    void free_ext_buffer(uint8* buf);

    ethernet_t* get_ethernet();
    arp_t*      get_arp();
    ip_t*       get_ip();
    icmp_t*     get_icmp();
    udp_t*      get_udp();
    tcp_t*      get_tcp();

    uint32 get_ipaddr();
    uint32 get_subnet_mask();
    uint32 get_gateway();
    uint32 get_dns_addr();

    static uint16 htons(uint16 n);
    static uint16 ntohs(uint16 n);
    static uint32 htonl(uint32 n);
    static uint32 ntohl(uint32 n);
    static uint32 make_ipaddr(uint8 ip0, uint8 ip1, uint8 ip2, uint8 ip3);
    static uint16 check_sum(uint8* data, uint32 len);
    static void dump_ip_addr(uint32 ip);
    static void dump_eth_addr(uint8 eth_addr[ETH_ADDR_LEN]);

private:
    list_t<net_buf_t *> m_net_buffers;
    list_t<uint8 *>     m_ext_buffers;

    ethernet_t          m_ethernet;
    arp_t               m_arp;
    ip_t                m_ip;
    icmp_t              m_icmp;
    udp_t               m_udp;
    tcp_t               m_tcp;

    uint32              m_ipaddr;
    uint32              m_subnet_mask;
    uint32              m_gateway;
    uint32              m_dns_addr;
};

#endif
