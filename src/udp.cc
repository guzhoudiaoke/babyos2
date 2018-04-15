/*
 * guzhoudiaoke@126.com
 * 2018-04-09
 */

#include "udp.h"
#include "babyos.h"
#include "net_buf.h"
#include "net.h"
#include "dns.h"
#include "socket_dgram.h"

void udp_pseudo_hdr_t::init(uint32 src_ip, uint32 dst_ip, uint8 protocol, uint16 udp_len)
{
    m_src_ip    = src_ip;
    m_dst_ip    = dst_ip;
    m_zero      = 0;
    m_protocol  = protocol;
    m_udp_len   = udp_len;
}

void udp_hdr_t::init(uint16 src, uint16 dest, uint16 len, uint16 checksum)
{
    m_src_port  = src;
    m_dst_port  = dest;
    m_length    = len;
    m_check_sum = checksum;
}

/***************************************************************/


int32 udp_t::init()
{
}

int32 udp_t::transmit(uint32 dst_ip, uint16 src_port, uint16 dst_port, uint8* data, uint32 len)
{
    uint16 total = len + sizeof(udp_hdr_t);

    udp_pseudo_hdr_t pseudo_hdr;
    pseudo_hdr.init(net_t::htonl(os()->get_net()->get_ipaddr()),
                    net_t::htonl(dst_ip),
                    ip_t::PROTO_UDP,
                    net_t::htons(total));

    //console()->kprintf(GREEN, "pseudo: %u %u %u %u %u\n", pseudo_hdr.m_src_ip, pseudo_hdr.m_dst_ip,
    //        pseudo_hdr.m_zero, pseudo_hdr.m_protocol, pseudo_hdr.m_udp_len);

    udp_hdr_t hdr;
    hdr.init(net_t::htons(src_port), 
             net_t::htons(dst_port), 
             net_t::htons(total), 
             0);

    //console()->kprintf(CYAN, "udp transmit, to ip: ");
    //net_t::dump_ip_addr(dst_ip);
    //console()->kprintf(CYAN, ", src port: %u, dst_port: %u, data len: %u, total: %u\n", 
    //        src_port, dst_port, len, total);

    net_buf_t* buffer = os()->get_net()->alloc_net_buffer(total + sizeof(pseudo_hdr));
    if (buffer == NULL) {
        return -1;
    }

    buffer->append(&pseudo_hdr, sizeof(udp_pseudo_hdr_t));
    buffer->append(&hdr, sizeof(udp_hdr_t));
    buffer->append(data, len);

    udp_hdr_t *header = (udp_hdr_t *) (buffer->get_data() + sizeof(udp_pseudo_hdr_t));
    header->m_check_sum = net_t::check_sum(buffer->get_data(), buffer->get_data_len());
    //console()->kprintf(WHITE, "check sum data len: %u\n", buffer->get_data_len());
    //for (int i = 0; i < buffer->get_data_len(); i++) {
    //    console()->kprintf(YELLOW, "%2x ", buffer->get_data()[i]);
    //}
    //console()->kprintf(YELLOW, "\n");

    buffer->pop_front(sizeof(udp_pseudo_hdr_t));
    os()->get_net()->get_ip()->transmit(dst_ip, buffer->get_data(), buffer->get_data_len(), ip_t::PROTO_UDP);

    return 0;
}

int32 udp_t::receive(net_buf_t* buf, uint32 ip)
{
    udp_hdr_t* hdr = (udp_hdr_t *) buf->get_data();
    udp_pseudo_hdr_t pseudo_hdr;
    uint16 total = net_t::ntohs(hdr->m_length);
    pseudo_hdr.init(net_t::htonl(ip),
                    net_t::htonl(os()->get_net()->get_ipaddr()),
                    ip_t::PROTO_UDP,
                    net_t::htons(total));


    //console()->kprintf(GREEN, "pseudo: %u %u %u %u %u\n", pseudo_hdr.m_src_ip, pseudo_hdr.m_dst_ip,
    //        pseudo_hdr.m_zero, pseudo_hdr.m_protocol, pseudo_hdr.m_udp_len);

    net_buf_t* buffer = os()->get_net()->alloc_net_buffer(sizeof(pseudo_hdr) + buf->get_data_len());
    if (buffer == NULL) {
        return -1;
    }

    //console()->kprintf(CYAN, "udp receive, from ip: ");
    //net_t::dump_ip_addr(ip);
    //console()->kprintf(CYAN, ", src port: %u, dst_port: %u, total len: %u\n", 
    //        net_t::ntohs(hdr->m_src_port), net_t::ntohs(hdr->m_dst_port), total);

    buffer->append(&pseudo_hdr, sizeof(udp_pseudo_hdr_t));
    buffer->append(buf->get_data(), total);
    uint16 check_sum = net_t::check_sum(buffer->get_data(), buffer->get_data_len());
    //console()->kprintf(WHITE, "check sum data len: %u\n", buffer->get_data_len());
    //for (int i = 0; i < buffer->get_data_len(); i++) {
    //    console()->kprintf(YELLOW, "%2x ", buffer->get_data()[i]);
    //}
    //console()->kprintf(YELLOW, "\n");

    os()->get_net()->free_net_buffer(buffer);
    if (check_sum != 0) {
        //console()->kprintf(RED, "receive a UDP packet, but check sum is wrong\n");
        return -1;
    }

    buf->pop_front(sizeof(udp_hdr_t));
    return socket_dgram_t::dgram_net_receive(buf, ip, net_t::ntohs(hdr->m_src_port), 
            os()->get_net()->get_ipaddr(), net_t::ntohs(hdr->m_dst_port));
}

