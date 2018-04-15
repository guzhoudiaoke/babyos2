/*
 * guzhoudiaoke@126.com
 * 2018-04-07
 */

#ifndef _SOCK_ADDR_H_
#define _SOCK_ADDR_H_

#include "types.h"

#define MAX_LOCAL_PATH      108

class sock_addr_t {
public:
    uint16 m_family;        /* address family, AF_xxx */
};

/* socket local */
class sock_addr_local_t : public sock_addr_t {
public:
    bool operator == (const sock_addr_local_t& addr);
    char m_path[MAX_LOCAL_PATH];
};


/* socket inet */
class sock_addr_inet_t : public sock_addr_t {
public:
    static const uint32 INADDR_ANY = 0;
    
    bool operator == (const sock_addr_inet_t& addr);
    uint32 m_ip;
    uint16 m_port;
};


#endif
