/*
 * 2018-04-21
 * guzhoudiaoke@126.com
 */

#ifndef _TCP_H_
#define _TCP_H_

#include "types.h"
#include "net_buf.h"

class tcp_pseudo_hdr_t {
public:
    void init(uint32 src_ip, uint32 dst_ip, uint8 protocol, uint16 tcp_len);

    uint32 m_src_ip;
    uint32 m_dst_ip;
    uint8  m_zero;
    uint8  m_protocol;
    uint16 m_tcp_len;
};

class tcp_hdr_flag_t {
public:
    uint8 m_fin:1;
    uint8 m_syn:1;
    uint8 m_pst:1;
    uint8 m_psh:1;
    uint8 m_ack:1;
    uint8 m_urg:1;
    uint8 m_ece:1;
    uint8 m_cwr:1;

    void init();
};

class tcp_hdr_t {
public:
    uint16 m_src_port;            /* source port number */
    uint16 m_dst_port;            /* destination port number */
    uint32 m_seq_no;
    uint32 m_ack_no;
    uint8  m_reserved:4;
    uint8  m_hdr_len:4;
    union {
        tcp_hdr_flag_t  m_flags;
        uint8           m_flags_val;
    };
    uint16 m_window;
    uint16 m_check_sum;
    uint16 m_urg_pointer;
};

class tcp_t {
public:
    int32 transmit(uint32 dst_ip, uint16 src_port, uint16 dst_port, uint32 seq_no, uint32 ack_no, 
            tcp_hdr_flag_t flag, uint8* data, uint32 len);
    int32 receive(net_buf_t* buf, uint32 ip);

private:
};

#endif
