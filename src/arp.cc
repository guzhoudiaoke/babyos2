/*
 * guzhoudiaoke@126.com
 * 2018-3-18
 */

#include "arp.h"
#include "string.h"
#include "babyos.h"

static void arp_timeout(uint32 data)
{
    os()->get_net()->get_arp()->request_timeout(data);
}


void arp_hdr_s::init(uint16 hw_type, uint16 proto_type, uint8 hw_len, uint8 proto_len,
        uint16 opcode, uint8 src_hw_addr[ETH_ADDR_LEN], uint32 src_proto_addr,
        uint8 tgt_hw_addr[ETH_ADDR_LEN], uint32 tgt_proto_addr)
{
    m_hardware_type = hw_type;
    m_protocol_type = proto_type;
    m_hardware_len  = hw_len;
    m_protocol_len  = proto_len;
    m_opcode        = opcode;
    m_source_protocol_addr = src_proto_addr;
    m_target_protocol_addr = tgt_proto_addr;

    memcpy(m_source_hw_addr, src_hw_addr, ETH_ADDR_LEN);
    memcpy(m_target_hw_addr, tgt_hw_addr, ETH_ADDR_LEN);
}

void arp_cache_line_s::init(uint32 ip, uint8 mac[ETH_ADDR_LEN])
{
    m_ip_addr = ip;
    memcpy(m_mac_addr, mac, ETH_ADDR_LEN);
}

bool arp_cache_line_s::operator == (const arp_cache_line_t& line)
{
    return (m_ip_addr == line.m_ip_addr);
}

bool arp_cache_line_s::operator != (const arp_cache_line_t& line)
{
    return (m_ip_addr != line.m_ip_addr);
}

void arp_queue_node_s::init(uint32 ip, uint32 retry, timer_t* timer)
{
    m_ip_addr = ip;
    m_retry_times = retry;
    m_buffers.init(os()->get_obj_pool_of_size());
    m_timer = timer;
}

/***************************************************************************/

void arp_t::init()
{
    m_arp_cache.init(os()->get_obj_pool_of_size());
    m_arp_wait_queue.init(os()->get_obj_pool_of_size());
}

void arp_t::request(uint32 ip)
{
    static uint8 empty_mac_addr[ETH_ADDR_LEN] = { 0x0 };
    static uint8 broadcast_mac_addr[ETH_ADDR_LEN] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

    arp_hdr_t hdr;
    hdr.init(net_t::ntohs(0x0001),                          /* ethernet */
             net_t::ntohs(PROTO_IP),                        /* IPv4 */
             0x06,                                          /* ethernet addr len */
             0x04,                                          /* IPv4 len */
             net_t::ntohs(ARP_OP_REQUEST),                  /* op */
             os()->get_net()->get_ethernet()->get_addr(),   /* my mac addr */
             net_t::ntohl(os()->get_net()->get_ipaddr()),   /* my ip addr */
             empty_mac_addr,                                /* 0 */
             net_t::ntohl(ip)                               /* which IP I want to know it's mac addr */
    );

    uint32 len = sizeof(arp_hdr_t);
    net_buf_t* buf = os()->get_net()->alloc_net_buffer(len);
    if (buf != NULL) {
        buf->append(&hdr, len);

        os()->get_net()->get_ethernet()->transmit(broadcast_mac_addr, PROTO_ARP, buf->get_data(), buf->get_data_len());
        os()->get_net()->free_net_buffer(buf);
    }
}

bool arp_t::add_to_cache(uint32 ip, uint8 eth_addr[ETH_ADDR_LEN])
{
    arp_cache_line_t cache_line;
    cache_line.init(ip, eth_addr);

    list_t<arp_cache_line_t>::iterator it = m_arp_cache.find(cache_line);
    if (it == m_arp_cache.end()) {
        m_arp_cache.push_front(cache_line);
    }
}

void arp_t::receive(uint8 ether_addr[ETH_ADDR_LEN], net_buf_t* buf)
{
    if (buf->get_data_len() < sizeof(arp_hdr_t)) {
        return;
    }

    arp_hdr_t* hdr = (arp_hdr_t *) buf->get_data();
    uint32 ip_src = net_t::htonl(hdr->m_source_protocol_addr);
    uint8* ip_bytes_src = (uint8 *) (&ip_src);

    switch (net_t::htons(hdr->m_opcode)) {
    case ARP_OP_REQUEST:
    {
        uint32 ip_target = net_t::htonl(hdr->m_target_protocol_addr);
        uint8* ip_bytes_tgt = (uint8 *) (&ip_target);
        //console()->kprintf(CYAN, "%d.%d.%d.%d want to know who have IP: %d.%d.%d.%d\n",
        //    ip_bytes_src[3], ip_bytes_src[2], ip_bytes_src[1], ip_bytes_src[0],
        //    ip_bytes_tgt[3], ip_bytes_tgt[2], ip_bytes_tgt[1], ip_bytes_tgt[0]);

        if (ip_target == os()->get_net()->get_ipaddr()) {
            hdr->init(net_t::ntohs(0x0001),                         /* ethernet */
                      net_t::ntohs(PROTO_IP),                       /* IPv4 */
                      0x06,                                         /* ethernet addr len */
                      0x04,                                         /* IPv4 len */
                      net_t::ntohs(ARP_OP_REPLY),                   /* op */
                      os()->get_net()->get_ethernet()->get_addr(),  /* my mac addr */
                      net_t::ntohl(os()->get_net()->get_ipaddr()),  /* my ip addr */
                      ether_addr,                                   /* target ether addr */
                      net_t::ntohl(ip_src)                          /* target protocol addr */
            );

            //console()->kprintf(GREEN, "arp reply to ip: ");
            //os()->get_net()->dump_ip_addr(ip_src);
            //console()->kprintf(GREEN, " \n");
            os()->get_net()->get_ethernet()->transmit(ether_addr, PROTO_ARP, 
                    buf->get_data(), buf->get_data_len());
        }
        break;
    }
    case ARP_OP_REPLY:
    {
        uint8 ether_src[ETH_ADDR_LEN];
        memcpy(ether_src, hdr->m_source_hw_addr, ETH_ADDR_LEN);
        console()->kprintf(YELLOW, "arp reply [%d.%d.%d.%d] -> [%2x:%2x:%2x:%2x:%2x:%2x]\n",
            ip_bytes_src[3], ip_bytes_src[2], ip_bytes_src[1], ip_bytes_src[0], 
            ether_src[0], ether_src[1], ether_src[2], ether_src[3], ether_src[4], ether_src[5]);

        add_to_cache(ip_src, ether_src);
        process_wait_queue(ip_src, ether_src);
        break;
    }
    default:
        break;
    }
}

bool arp_t::lookup_cache(uint32 ip, uint8 eth_addr[ETH_ADDR_LEN])
{
    list_t<arp_cache_line_t>::iterator it = m_arp_cache.begin();
    while (it != m_arp_cache.end()) {
        if ((*it).m_ip_addr == ip) {
            break;
        }
        it++;
    }

    if (it == m_arp_cache.end()) {
        return false;
    }

    memcpy(eth_addr, (*it).m_mac_addr, ETH_ADDR_LEN);
    return true;
}

void arp_t::add_to_wait_queue(uint32 ip, net_buf_t* buf)
{
    console()->kprintf(PINK, "add ip: ");
    os()->get_net()->dump_ip_addr(ip);
    console()->kprintf(PINK, " buf: 0x%x\n", buf);

    spinlock_t* lock = m_arp_wait_queue.get_lock();
    uint32 flag;
    lock->lock_irqsave(flag);

    list_t<arp_queue_node_t>::iterator it = m_arp_wait_queue.begin();
    while (it != m_arp_wait_queue.end()) {
        if ((*it).m_ip_addr == ip) {
            break;
        }
        it++;
    }

    /* find the ip */
    if (it != m_arp_wait_queue.end()) {
        if ((*it).m_buffers.find(buf) == (*it).m_buffers.end()) {
            (*it).m_buffers.push_back(buf);
        }
    }
    else {
        timer_t* timer = (timer_t *) os()->get_obj_pool(TIMER_POOL)->alloc_from_pool();
        timer->init(ARP_TIMEOUT, ip, arp_timeout);

        arp_queue_node_t node;
        node.init(ip, ARP_RETRY, timer);
        node.m_buffers.push_back(buf);
        m_arp_wait_queue.push_back(node);

        os()->get_net()->arp_request(ip);
        os()->get_timer_mgr()->add_timer(timer);
    }

    it = m_arp_wait_queue.begin();
    while (it != m_arp_wait_queue.end()) {
        console()->kprintf(PINK, "ip: ");
        os()->get_net()->dump_ip_addr((*it).m_ip_addr);

        list_t<net_buf_t *>::iterator it_buffer = (*it).m_buffers.begin();
        while (it_buffer != (*it).m_buffers.end()) {
            console()->kprintf(PINK, " buf: 0x%x, ", (*it_buffer));
            it_buffer++;
        }
        console()->kprintf(PINK, "\n");
        it++;
    }

    lock->unlock_irqrestore(flag);
}

void arp_t::process_wait_queue(uint32 ip, uint8 eth_addr[ETH_ADDR_LEN])
{
    spinlock_t* lock = m_arp_wait_queue.get_lock();
    uint32 flag;
    lock->lock_irqsave(flag);

    list_t<arp_queue_node_t>::iterator it = m_arp_wait_queue.begin();
    while (it != m_arp_wait_queue.end()) {
        if ((*it).m_ip_addr == ip) {
            break;
        }
        it++;
    }

    if (it != m_arp_wait_queue.end()) {
        list_t<net_buf_t *>::iterator it_buffer = (*it).m_buffers.begin();
        while (it_buffer != (*it).m_buffers.end()) {
            net_buf_t* buffer = *it_buffer;
            os()->get_net()->get_ethernet()->transmit(eth_addr, PROTO_IP, buffer->get_data(), buffer->get_data_len());
            os()->get_net()->free_net_buffer(buffer);
            it_buffer = (*it).m_buffers.erase(it_buffer);;
        }

        timer_t* timer = (*it).m_timer;
        os()->get_timer_mgr()->remove_timer(timer);
        os()->get_obj_pool(TIMER_POOL)->free_object(timer);
        m_arp_wait_queue.erase(it);
    }

    lock->unlock_irqrestore(flag);
}

void arp_t::request_timeout(uint32 ip)
{
    console()->kprintf(GREEN, "arp request of ip: ");
    os()->get_net()->dump_ip_addr(ip);
    console()->kprintf(GREEN, " timeout.\n");

    spinlock_t* lock = m_arp_wait_queue.get_lock();
    uint32 flag;
    lock->lock_irqsave(flag);

    list_t<arp_queue_node_t>::iterator it = m_arp_wait_queue.begin();
    while (it != m_arp_wait_queue.end()) {
        if ((*it).m_ip_addr == ip) {
            break;
        }
        it++;
    }

    if (it != m_arp_wait_queue.end()) {
        timer_t* timer = (*it).m_timer;
        os()->get_timer_mgr()->remove_timer(timer);

        if ((*it).m_retry_times-- > 0) {
            timer->init(ARP_TIMEOUT, (*it).m_ip_addr, arp_timeout);
            os()->get_timer_mgr()->add_timer(timer);
            os()->get_net()->arp_request(ip);
            console()->kprintf(YELLOW, "arp retry, left retry time: %u\n", (*it).m_retry_times);
        }
        else {
            list_t<net_buf_t *>::iterator it_buffer = (*it).m_buffers.begin();
            while (it_buffer != (*it).m_buffers.end()) {
                net_buf_t* buffer = *it_buffer;
                os()->get_net()->free_net_buffer(buffer);
                it_buffer = (*it).m_buffers.erase(it_buffer);;
            }

            os()->get_obj_pool(TIMER_POOL)->free_object(timer);
            m_arp_wait_queue.erase(it);
        }
    }

    lock->unlock_irqrestore(flag);
}

