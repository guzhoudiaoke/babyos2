/*
 * guzhoudiaoke@126.com
 * 2018-04-09
 */

#ifndef _DNS_H_
#define _DNS_H_

#include "types.h"
#include "net_buf.h"

#define MAX_DNS_PACKET_SIZE 512
#define MAX_LABEL_LEN   63

#define RR_TYPE_A       1   /* ipv4 */
#define RR_TYPE_CNAME   5   /* map a name to another */

class dns_hdr_flag_t {
public:
    uint16  m_flags_rcode :  4;     /* response code */
    uint16  m_flags_z :      3;     /* zero, not used */
    uint16  m_flags_ra :     1;     /* recursion available */
    uint16  m_flags_rd :     1;     /* recursion desired */
    uint16  m_flags_tc :     1;     /* truncated */
    uint16  m_flags_aa :     1;     /* authoritative answer */
    uint16  m_flags_opcode : 4;     /* 0: standard query, 4: notify, 5: update, other not used */
    uint16  m_flags_qr :     1;     /* 0: query, 1: reply */
};

class dns_hdr_t {
public:
    uint16  m_transaction_id;
    union {
        dns_hdr_flag_t  m_flags;
        uint16          m_flags_val;
    };
    uint16  m_qd_count;
    uint16  m_an_count;
    uint16  m_ns_count;
    uint16  m_ar_count;
};

class dns_t {
public:
    static int32 parse_name(const char* name, net_buf_t* buffer);
    static int32 query(const char* name);
    static int32 receive(net_buf_t* buffer);

    static uint32 resolve_name(const uint8* dns_data, const uint8* data, char* name);
    static int32  resolve(net_buf_t* buffer);
};


#endif
