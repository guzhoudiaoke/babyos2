/*
 * guzhoudiaoke@126.com
 * 2018-3-18
 */

#ifndef _ARP_H_
#define _ARP_H_

#include "types.h"
#include "ethernet.h"
#include "list.h"
#include "timer.h"
#include "net_buf.h"

#define ARP_OP_REQUEST  1
#define ARP_OP_REPLY    2

#define ARP_TIMEOUT     100     /* 100 ticks */
#define ARP_RETRY       3

#pragma pack(push, 1)
typedef struct arp_hdr_s {
    uint16  m_hardware_type;
    uint16  m_protocol_type;
    uint8   m_hardware_len;
    uint8   m_protocol_len;
    uint16  m_opcode;
    uint8   m_source_hw_addr[ETH_ADDR_LEN];
    uint32  m_source_protocol_addr;
    uint8   m_target_hw_addr[ETH_ADDR_LEN];
    uint32  m_target_protocol_addr;

    void init(uint16 hw_type, uint16 proto_type, uint8 hw_len, uint8 proto_len,
            uint16 opcode, uint8 src_hw_addr[ETH_ADDR_LEN], uint32 src_proto_addr,
            uint8 tgt_hw_addr[ETH_ADDR_LEN], uint32 tgt_proto_addr);
} arp_hdr_t;
#pragma pack(pop)

typedef struct arp_cache_line_s {
    uint32 m_ip_addr;
    uint8  m_mac_addr[ETH_ADDR_LEN];

    void init(uint32 ip, uint8 mac[ETH_ADDR_LEN]);
    bool operator == (const arp_cache_line_s& line);
    bool operator != (const arp_cache_line_s& line);
} arp_cache_line_t;

typedef struct arp_queue_node_s {
    uint32              m_ip_addr;
    uint32              m_retry_times;
    timer_t*            m_timer;
    list_t<net_buf_t *> m_buffers;

    void init(uint32 ip, uint32 retry, timer_t* timer);
} arp_queue_node_t;

class arp_t {
public:
    void init();
    void request(uint32 ip);
    void receive(uint8 ether_addr[ETH_ADDR_LEN], net_buf_t* buf);
    bool lookup_cache(uint32 ip, uint8 eth_addr[ETH_ADDR_LEN]);
    void add_to_wait_queue(uint32 ip, net_buf_t* buf);
    void request_timeout(uint32 ip);

private:
    bool add_to_cache(uint32 ip, uint8 eth_addr[ETH_ADDR_LEN]);
    void process_wait_queue(uint32 ip, uint8 eth_addr[ETH_ADDR_LEN]);

private:
    list_t<arp_cache_line_t>    m_arp_cache;
    list_t<arp_queue_node_t>    m_arp_wait_queue;
};

#endif
