/*
 * guzhoudiaoke@126.com
 * 2018-04-07
 */

#include "userlib.h"

uint32 parse_ip(const char* str)
{
    return 0;
}

void ping(uint32 ip)
{
    int sock_fd = userlib_t::socket(socket_t::AF_INET, 0, 0);
    if (sock_fd < 0) {
        userlib_t::printf("client create socket failed, error %u\n", sock_fd);
        return;
    }
    userlib_t::printf("client create socket success, fd: %u\n", sock_fd);

    sock_addr_inet_t dst_addr;
    dst_addr.m_family = socket_t::AF_LOCAL;
    dst_addr.m_ip = ip;
    dst_addr.m_port = 0;

    for (int i = 0; i < 4; i++) {
    }
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        userlib_t::printf("Usage: ping a.b.c.d\n");
        userlib_t::exit(0);
    }

    uint32 ip = parse_ip(argv[1]);
    ping(ip);


    userlib_t::exit(0);
    return 0;
}
