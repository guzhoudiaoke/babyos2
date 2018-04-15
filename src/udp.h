/*
 * guzhoudiaoke@126.com
 * 2018-04-09
 */

#ifndef _UDP_H_
#define _UDP_H_

#include "types.h"
#include "net_buf.h"

class udp_pseudo_hdr_t {
public:
    void init(uint32 src_ip, uint32 dst_ip, uint8 protocol, uint16 udp_len);

    uint32 m_src_ip;
    uint32 m_dst_ip;
    uint8  m_zero;
    uint8  m_protocol;
    uint16 m_udp_len;
};

class udp_hdr_t {
public:
    void init(uint16 src, uint16 dest, uint16 len, uint16 checksum);
    
    uint16 m_src_port;            /* source port number */
    uint16 m_dst_port;            /* destination port number */
    uint16 m_length;              /* length */
    uint16 m_check_sum;           /* checksum */
};

class udp_t {
public:
    int32 init();
    int32 transmit(uint32 dst_ip, uint16 src_port, uint16 dst_port, uint8* data, uint32 len);
    int32 receive(net_buf_t* buf, uint32 ip);
};

#endif

