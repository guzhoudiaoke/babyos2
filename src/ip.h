/*
 * guzhoudiaoke@126.com
 * 2018-03-21
 */

#ifndef _IP_H_
#define _IP_H_

#include "types.h"
#include "list.h"
#include "net_buf.h"

typedef struct ip_hdr_s {
    uint8   m_header_len:4;
    uint8   m_version:4;
    uint8   m_tos;          /* type of service */
    uint16  m_total_len;
    uint16  m_id;
    uint16  m_offset;       /* fragment offset field */
    uint8   m_ttl;          /* time to live */
    uint8   m_protocol;
    uint16  m_check_sum;
    uint32  m_src_ip;
    uint32  m_dst_ip;

    void init(uint8 hdr_len, uint8 ver, uint8 tos, uint16 total_len,
            uint16 id, uint16 offset, uint8 ttl, uint8 proto, uint16 check_sum, 
            uint32 src_ip, uint32 dst_ip);
} ip_hdr_t;


class ip_t {
public:
    enum e_protocol {
        PROTO_ICMP = 1,
        PROTO_TCP = 6,
        PROTO_UDP = 17,
        PROTO_RAW = 0xff
    };

public:
    void init();
    void transmit(uint32 ip, uint8* data, uint32 len, uint8 protocol);
    void receive(net_buf_t* buf);

private:
    bool is_broadcast(uint32 ip);
    bool check_ip(uint32 ip);
    bool is_same_subnet(uint32 ip, uint32 ip2, uint32 mask);

private:
    uint16                  m_next_id;
};

#endif
