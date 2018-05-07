/*
 * guzhoudiaoke@126.com
 * 2018-03-21
 */

#include "ip.h"
#include "babyos.h"
#include "net.h"
#include "string.h"
#include "socket_raw.h"

void ip_hdr_s::init(uint8 hdr_len, uint8 ver, uint8 tos, uint16 total_len,
        uint16 id, uint16 offset, uint8 ttl, uint8 proto, uint16 check_sum, 
        uint32 src_ip, uint32 dst_ip)
{
    m_header_len = hdr_len;
    m_version    = ver;
    m_tos        = tos;
    m_total_len  = total_len;
    m_id         = id;
    m_offset     = offset;
    m_ttl        = ttl;
    m_protocol   = proto;
    m_check_sum  = check_sum;
    m_src_ip     = src_ip;
    m_dst_ip     = dst_ip;
}

/**********************************************************/

void ip_t::init()
{
    m_next_id = 0;
}

bool ip_t::is_broadcast(uint32 ip)
{
    if (ip == 0xffffffff || ip == 0x00000000) {
        return true;
    }

    uint32 mask = os()->get_net()->get_subnet_mask();
    if ((ip & ~mask) == (0xffffffff & ~mask)) {
        return true;
    }

    return false;
}

bool ip_t::check_ip(uint32 ip)
{
    if (is_broadcast(ip)) {
        console()->kprintf(RED, "DO NOT support broadcast now\n");
        return false;
    }

    if (ip == os()->get_net()->get_ipaddr()) {
        console()->kprintf(RED, "DO NOT support loopback now\n");
        return false;
    }

    return true;
}

bool ip_t::is_same_subnet(uint32 ip1, uint32 ip2, uint32 mask)
{
    return (ip1 & mask) == (ip2 & mask);
}

void ip_t::transmit(uint32 ip, uint8* data, uint32 len, uint8 protocol)
{
    if (!check_ip(ip)) {
        return;
    }

    uint32 dst_ip = ip;
    if (!is_same_subnet(ip, os()->get_net()->get_ipaddr(), os()->get_net()->get_subnet_mask())) {
        /* if not in same subnet, send to gateway */
        console()->kprintf(PINK, "not in same subnet, send to gateway\n");
        dst_ip = os()->get_net()->get_gateway();
    }

    uint32 total = len + sizeof(ip_hdr_t);
    net_buf_t* buffer = os()->get_net()->alloc_net_buffer(total);
    if (buffer == NULL) {
        console()->kprintf(RED, "allocate net buffer failed!\n");
        return;
    }

    ip_hdr_t hdr;
    hdr.init(sizeof(ip_hdr_t) / 4,                          /* header len */
             0x4,                                           /* version */
             0,                                             /* tos */
             net_t::htons(total),                           /* total len */
             net_t::htons(m_next_id++),                     /* id */
             net_t::htons(0),                               /* offset, don't fragment */
             32,                                            /* ttl */
             protocol,                                      /* protocol */
             0,                                             /* check sum */
             net_t::htonl(os()->get_net()->get_ipaddr()),   /* source ip */
             net_t::htonl(ip));                             /* dest ip */

    /* calc check sum */
    hdr.m_check_sum = net_t::check_sum((uint8 *) (&hdr), sizeof(ip_hdr_t));

    buffer->append(&hdr, sizeof(ip_hdr_t));
    buffer->append(data, len);

    uint8 eth_addr[ETH_ADDR_LEN] = {0};
    if (os()->get_net()->get_arp()->lookup_cache(dst_ip, eth_addr)) {
        console()->kprintf(CYAN, "dest ip: ");
        net_t::dump_ip_addr(dst_ip);
        console()->kprintf(CYAN, " -> find mac addr of ip in arp cache\n");

        os()->get_net()->get_ethernet()->transmit(eth_addr, PROTO_IP, buffer->get_data(), buffer->get_data_len());
        os()->get_net()->free_net_buffer(buffer);
    }
    else {
        console()->kprintf(YELLOW, "not find mac addr of ip in arp cache\n");
        os()->get_net()->get_arp()->add_to_wait_queue(dst_ip, buffer);
    }
}

void ip_t::receive(net_buf_t* buf)
{
    ip_hdr_t* hdr = (ip_hdr_t *) buf->get_data();
    buf->pop_front(sizeof(ip_hdr_t));

    /* check sum */
    if (net_t::check_sum((uint8 *) hdr, sizeof(ip_hdr_t)) != 0) {
        //console()->kprintf(RED, "get a ip package, from: ");
        //net_t::dump_ip_addr(net_t::ntohl(hdr->m_src_ip));
        //console()->kprintf(RED, " but it's checksum is wrong, drop it\n");
        return;
    }

    /* for raw socket */
    if (socket_raw_t::raw_net_receive(buf, hdr->m_protocol, net_t::ntohl(hdr->m_src_ip)) == 0) {
        console()->kprintf(CYAN, "socket raw process a packet, protocol: %u\n", hdr->m_protocol);
    }

    switch (hdr->m_protocol) {
    case PROTO_RAW:
        console()->kprintf(YELLOW, "get a raw ip package, data: %s\n", buf->get_data());
        os()->get_net()->free_net_buffer(buf);
        break;
    case PROTO_ICMP:
        os()->get_net()->get_icmp()->receive(buf, net_t::ntohl(hdr->m_src_ip));
        os()->get_net()->free_net_buffer(buf);
        break;
    case PROTO_UDP:
        if (!os()->get_net()->get_udp()->receive(buf, net_t::ntohl(hdr->m_src_ip))) {
            os()->get_net()->free_net_buffer(buf);
        }
        break;
    case PROTO_TCP:
        if (!os()->get_net()->get_tcp()->receive(buf, net_t::ntohl(hdr->m_src_ip))) {
            os()->get_net()->free_net_buffer(buf);
        }
        break;
    default:
        //console()->kprintf(YELLOW, "get an ip package with protocol: %x, not support.\n", hdr->m_protocol);
        os()->get_net()->free_net_buffer(buf);
        break;
    }
}

