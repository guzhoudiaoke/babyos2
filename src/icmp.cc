/*
 * guzhoudiaoke@126.com
 * 2018-03-25
 */

#include "icmp.h"
#include "babyos.h"
#include "string.h"
#include "net.h"

void icmp_echo_hdr_t::init(uint8 type, uint8 code, uint16 check_sum, uint16 id, uint16 seq)
{
    m_type      = type;
    m_code      = code;
    m_check_sum = check_sum;
    m_id        = id;
    m_seq_no    = seq;
}

/***********************************************************************/

bool icmp_t::echo_request(uint32 ip, uint16 id, uint16 seq, uint8* data, uint32 len)
{
    uint32 total = len + sizeof(icmp_echo_hdr_t);
    net_buf_t* buffer = os()->get_net()->alloc_net_buffer(total);
    if (buffer == NULL) {
        console()->kprintf(RED, "ICMP echo_request, alloc net buffer failed, total: %u\n", total);
        return false;
    }

    console()->kprintf(GREEN, "send an icmp echo request to ip: ");
    net_t::dump_ip_addr(ip);
    console()->kprintf(GREEN, " seq: %u\n", seq);

    icmp_echo_hdr_t hdr;
    hdr.init(ECHO_REQUEST,          /* type */
             0,                     /* code */ 
             0,                     /* check sum */
             net_t::htons(id),      /* id */
             net_t::htons(seq));    /* seq no */
    hdr.m_check_sum = net_t::check_sum((uint8 *) &hdr, sizeof(icmp_echo_hdr_t));

    buffer->append(&hdr, sizeof(icmp_echo_hdr_t));
    if (data != NULL && len != 0) {
        buffer->append(data, len);
    }

    os()->get_net()->get_ip()->transmit(ip, buffer->get_data(), total, ip_t::PROTO_ICMP);
    os()->get_net()->free_net_buffer(buffer);
    return true;
}

bool icmp_t::echo_reply(uint32 ip, uint16 id, uint16 seq, uint8* data, uint32 len)
{
    uint32 total = len + sizeof(icmp_echo_hdr_t);
    net_buf_t* buffer = os()->get_net()->alloc_net_buffer(total);
    if (buffer == NULL) {
        console()->kprintf(RED, "ICMP echo_reply, alloc net buffer failed, total: %u\n", total);
        return false;
    }

    console()->kprintf(GREEN, "send an icmp echo reply to ip: ");
    net_t::dump_ip_addr(ip);
    console()->kprintf(GREEN, " seq: %u\n", seq);

    icmp_echo_hdr_t hdr;
    hdr.init(ECHO_REPLY,            /* type */
             0,                     /* code */ 
             0,                     /* check sum */
             net_t::htons(id),      /* id */
             net_t::htons(seq));    /* seq no */
    hdr.m_check_sum = net_t::check_sum((uint8 *) &hdr, sizeof(icmp_echo_hdr_t));

    buffer->append(&hdr, sizeof(icmp_echo_hdr_t));
    if (data != NULL && len != 0) {
        buffer->append(data, len);
    }

    os()->get_net()->get_ip()->transmit(ip, buffer->get_data(), total, ip_t::PROTO_ICMP);
    os()->get_net()->free_net_buffer(buffer);
    return true;
}

void icmp_t::echo_request_receive(net_buf_t* buf, uint32 ip)
{
    icmp_echo_hdr_t* hdr = (icmp_echo_hdr_t *) buf->get_data();
    uint16 check_sum = net_t::check_sum(buf->get_data(), buf->get_data_len());
    if (check_sum != 0) {
        console()->kprintf(RED, "receive an icmp echo request, but checksum is error: %x.\n", check_sum);
        return;
    }

    console()->kprintf(GREEN, "receive an icmp echo request from ip: ");
    net_t::dump_ip_addr(ip);
    console()->kprintf(GREEN, " seq: %u\n", net_t::ntohs(hdr->m_seq_no));
    echo_reply(ip, net_t::ntohs(hdr->m_id), net_t::ntohs(hdr->m_seq_no), NULL, 0);
}

void icmp_t::echo_reply_receive(net_buf_t* buf, uint32 ip)
{
    icmp_echo_hdr_t* hdr = (icmp_echo_hdr_t *) buf->get_data();
    uint16 check_sum = net_t::check_sum(buf->get_data(), buf->get_data_len());
    if (check_sum != 0) {
        console()->kprintf(RED, "receive an icmp echo reply, but checksum is error: %x.\n", check_sum);
        return;
    }

    //console()->kprintf(WHITE, "receive an icmp echo reply from ip: ");
    //net_t::dump_ip_addr(ip);
    //console()->kprintf(WHITE, " seq: %u\n", net_t::ntohs(hdr->m_seq_no));
}

void icmp_t::receive(net_buf_t* buf, uint32 ip)
{
    uint8 type = *(uint8 *) buf->get_data();
    switch (type) {
    case ECHO_REQUEST:
        echo_request_receive(buf, ip);
        break;
    case ECHO_REPLY:
        echo_reply_receive(buf, ip);
        break;
    default:
        console()->kprintf(RED, "receive an icmp package, but not support the type %x now.\n", type);
        break;
    }
}

