/*
 * guzhoudiaoke@126.com
 * 2018-03-25
 */

#ifndef _ICMP_H_
#define _ICMP_H_

#include "types.h"
#include "net_buf.h"

class icmp_echo_hdr_t {
public:
    void init(uint8 type, uint8 code, uint16 check_sum, uint16 id, uint16 seq);

public:
    uint8  m_type;
    uint8  m_code;
    uint16 m_check_sum;
    uint16 m_id;
    uint16 m_seq_no;
};

class icmp_t {
public:
    enum e_type {
        ECHO_REPLY = 0,
        ECHO_REQUEST = 8,
    };

public:
    bool echo_request(uint32 ip, uint16 id, uint16 seq, uint8* data, uint32 len);
    bool echo_reply(uint32 ip, uint16 id, uint16 seq, uint8* data, uint32 len);
    void receive(net_buf_t* buf, uint32 ip);

private:
    void echo_request_receive(net_buf_t* buf, uint32 ip);
    void echo_reply_receive(net_buf_t* buf, uint32 ip);

};

#endif
