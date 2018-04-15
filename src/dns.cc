/*
 * guzhoudiaoke@126.com
 * 2018-04-09
 */

#include "dns.h"
#include "babyos.h"
#include "string.h"


int32 dns_t::parse_name(const char* name, net_buf_t* buffer)
{
    char tmp[256] = {0};
    const char* p = name;

    while (*p != '\0') {
        const char* begin = p;
        while (*p != '\0' && *p != '.') {
            p++;
        }
        if (p - begin > MAX_LABEL_LEN) {
            return -1;
        }
        
        uint8 count = p - begin;
        buffer->append(&count, 1);
        buffer->append(begin, count);

        strncpy(tmp, begin, count);
        if (*p == '\0') {
            count = 0;
            buffer->append(&count, 1);
            break;
        }
        p++;
    }

    return 0;
}

int32 dns_t::query(const char* name)
{
    console()->kprintf(GREEN, "dns query: %s\n", name);

    dns_hdr_t hdr;
    hdr.m_transaction_id         = net_t::htons(0x7844); 
    hdr.m_flags.m_flags_qr       = 0;
    hdr.m_flags.m_flags_opcode   = 0;
    hdr.m_flags.m_flags_aa       = 0;
    hdr.m_flags.m_flags_tc       = 0;
    hdr.m_flags.m_flags_rd       = 1;
    hdr.m_flags.m_flags_ra       = 0;
    hdr.m_flags.m_flags_z        = 0;
    hdr.m_flags.m_flags_rcode    = 0;
    hdr.m_flags_val              = net_t::htons(hdr.m_flags_val);
    hdr.m_qd_count               = net_t::htons(1);
    hdr.m_an_count               = net_t::htons(0);
    hdr.m_ns_count               = net_t::htons(0);
    hdr.m_ar_count               = net_t::htons(0);

    net_buf_t* buffer = os()->get_net()->alloc_net_buffer(512);
    if (buffer == NULL) {
        return -1;
    }

    buffer->append(&hdr, sizeof(dns_hdr_t));
    parse_name(name, buffer);


    uint16 query_type = net_t::htons(1);            /* host address */
    uint16 query_class = net_t::htons(1);
    buffer->append(&query_type, sizeof(query_type));
    buffer->append(&query_class,sizeof(query_class));

    uint32 dns_ip = os()->get_net()->get_dns_addr();
    os()->get_net()->get_udp()->transmit(dns_ip, 50150, 53, buffer->get_data(), buffer->get_data_len());

    return 0;
}

uint32 dns_t::resolve_name(const uint8* dns_data, const uint8* data, char* name)
{
    const uint8* start = data;
    uint8 len = (uint8) *data++;
    uint32 total = 0;
    while (len != 0) {
        if (len <= MAX_LABEL_LEN) {
            memcpy(name, data, len);
            name += len;
            data += len;
        }
        else {
            uint16 offset = ((len & 0x3f) << 8 | *data++);
            const uint8* p = dns_data + offset;
            uint32 count = resolve_name(dns_data, p, name);
            name += strlen(name);
            break;
        }

        len = *data++;
        if (len != 0) {
            *name++ = '.';
        }
    }

    *name++ = '\0';
    return data - start;
}

int32 dns_t::resolve(net_buf_t* buffer)
{
    dns_hdr_t* hdr = (dns_hdr_t *) buffer->get_data();

    uint16 query_count = net_t::ntohs(hdr->m_qd_count);
    uint16 answer_count = net_t::ntohs(hdr->m_an_count);
    console()->kprintf(CYAN, "ID: 0x%x, flags: 0x%x, questions num: %u, answer num: %u\n", 
            net_t::ntohs(hdr->m_transaction_id), net_t::ntohs(hdr->m_flags_val), 
            query_count, answer_count);

    uint8* dns_data = buffer->get_data();
    uint8* p = dns_data + sizeof(dns_hdr_t);
    char name[MAX_DNS_PACKET_SIZE] = {0};

    console()->kprintf(GREEN, "queries:\n");
    for (int i = 0; i < query_count; i++) {
        memset(name, 0, MAX_DNS_PACKET_SIZE);
        p += resolve_name(dns_data, p, name);
        uint16* query_type = (uint16 *) p;
        uint16* query_class = (uint16 *) (query_type + 1);
        console()->kprintf(CYAN, "%s, type 0x%4x, class 0x%4x\n", name, 
                net_t::ntohs(*query_type), net_t::ntohs(*query_class));
        p = (uint8 *) (query_class + 1);
    }

    console()->kprintf(PINK, "answers:\n");
    for (int i = 0; i < answer_count; i++) {
        memset(name, 0, MAX_DNS_PACKET_SIZE);
        p += resolve_name(dns_data, p, name);

        uint16* ans_type = (uint16 *) p;
        uint16* ans_class = (uint16 *) (ans_type + 1);
        uint32* ttl = (uint32 *) (ans_class + 1);
        uint16* data_len = (uint16 *) (ttl + 1);
        console()->kprintf(YELLOW, "%s, type 0x%4x, class 0x%4x, 0x%8x, 0x%4x -> ",  name, 
                net_t::ntohs(*ans_type), net_t::ntohs(*ans_class), 
                net_t::ntohl(*ttl), net_t::ntohs(*data_len));

        p = (uint8 *) (data_len + 1);
        if (net_t::ntohs(*ans_type) == RR_TYPE_A) {
            uint32* ip = (uint32 *) p;
            net_t::dump_ip_addr(net_t::ntohl(*ip));
        }
        else if (net_t::ntohs(*ans_type) == RR_TYPE_CNAME) {
        memset(name, 0, MAX_DNS_PACKET_SIZE);
            resolve_name(dns_data, p, name);
            console()->kprintf(PINK, "%s", name);
        }
        p += net_t::ntohs(*data_len);
        console()->kprintf(YELLOW, "\n");
    }

    return 0;
}

int32 dns_t::receive(net_buf_t* buffer)
{
    console()->kprintf(GREEN, "receive a dns reply. resolve:\n");
    resolve(buffer);

    return 0;
}

