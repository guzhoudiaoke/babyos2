/*
 * 2018-04-21
 * guzhoudiaoke@126.com
 */

#include "tcp.h"
#include "babyos.h"
#include "net_buf.h"
#include "net.h"
#include "dns.h"
#include "socket_stream.h"

const uint16 c_min_length_tcp_data = 26;

void tcp_pseudo_hdr_t::init(uint32 src_ip, uint32 dst_ip, uint8 protocol, uint16 tcp_len)
{
    m_src_ip    = src_ip;
    m_dst_ip    = dst_ip;
    m_zero      = 0;
    m_protocol  = protocol;
    m_tcp_len   = tcp_len;
}

void tcp_hdr_flag_t::init()
{
    m_cwr = 0;
    m_ece = 0;
    m_urg = 0;
    m_ack = 0;
    m_psh = 0;
    m_pst = 0;
    m_syn = 0;
    m_fin = 0;
}

/******************************************************************/

int32 tcp_t::transmit(uint32 dst_ip, uint16 src_port, uint16 dst_port, uint32 seq_no, uint32 ack_no, 
        tcp_hdr_flag_t flag, uint8* data, uint32 len)
{
    uint16 tcp_len = len + sizeof(tcp_hdr_t);
    uint16 total = tcp_len;
    if (total < c_min_length_tcp_data) {
        total = c_min_length_tcp_data;
    }

    tcp_pseudo_hdr_t pseudo_hdr;
    pseudo_hdr.init(net_t::htonl(os()->get_net()->get_ipaddr()),
                    net_t::htonl(dst_ip),
                    ip_t::PROTO_TCP,
                    net_t::htons(total));

    tcp_hdr_t hdr;
    hdr.m_src_port    = net_t::htons(src_port);
    hdr.m_dst_port    = net_t::htons(dst_port);
    hdr.m_seq_no      = net_t::htonl(seq_no);
    hdr.m_ack_no      = net_t::htonl(ack_no);
    hdr.m_hdr_len     = sizeof(tcp_hdr_t) / 4;
    hdr.m_reserved    = 0;
    hdr.m_flags       = flag;
    hdr.m_window      = net_t::htons(1);
    hdr.m_check_sum   = 0;
    hdr.m_urg_pointer = 0;

    net_buf_t* buffer = os()->get_net()->alloc_net_buffer(total + sizeof(pseudo_hdr));
    if (buffer == NULL) {
        return -1;
    }

    buffer->append(&pseudo_hdr, sizeof(tcp_pseudo_hdr_t));
    buffer->append(&hdr, sizeof(tcp_hdr_t));
    if (data != NULL && len != 0) {
        buffer->append(data, len);
    }
    if (total > tcp_len) {
        buffer->append_zero(total - tcp_len);
    }

    tcp_hdr_t *header = (tcp_hdr_t *) (buffer->get_data() + sizeof(tcp_pseudo_hdr_t));
    header->m_check_sum = net_t::check_sum(buffer->get_data(), buffer->get_data_len());

    buffer->pop_front(sizeof(tcp_pseudo_hdr_t));
    os()->get_net()->get_ip()->transmit(dst_ip, buffer->get_data(), buffer->get_data_len(), ip_t::PROTO_TCP);

    return 0;
}

int32 tcp_t::receive(net_buf_t* buf, uint32 ip)
{
    tcp_hdr_t* hdr = (tcp_hdr_t *) buf->get_data();
    tcp_pseudo_hdr_t pseudo_hdr;
    uint16 total = buf->get_data_len();
    
    pseudo_hdr.init(net_t::htonl(ip),
                    net_t::htonl(os()->get_net()->get_ipaddr()),
                    ip_t::PROTO_TCP,
                    net_t::htons(total));


    net_buf_t* buffer = os()->get_net()->alloc_net_buffer(sizeof(pseudo_hdr) + buf->get_data_len());
    if (buffer == NULL) {
        return -1;
    }

    buffer->append(&pseudo_hdr, sizeof(tcp_pseudo_hdr_t));
    buffer->append(buf->get_data(), total);
    uint16 check_sum = net_t::check_sum(buffer->get_data(), buffer->get_data_len());

    os()->get_net()->free_net_buffer(buffer);
    if (check_sum != 0) {
        console()->kprintf(RED, "receive a TCP packet, but check sum is wrong\n");
        return -1;
    }

    console()->kprintf(GREEN, "receive a TCP packet from ip: ");
    net_t::dump_ip_addr(ip);
    console()->kprintf(GREEN, " port: %u, syn: %u, ack: %u, fin: %u\n", net_t::ntohs(hdr->m_src_port),
            hdr->m_flags.m_syn, hdr->m_flags.m_ack, hdr->m_flags.m_fin);

    buf->pop_front(sizeof(tcp_hdr_t));
    return socket_stream_t::stream_net_receive(hdr, buf, ip, os()->get_net()->get_ipaddr());
}

