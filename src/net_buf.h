/*
 * guzhoudiaoke@126.com
 * 2018-04-08
 */

#ifndef _NET_BUF_H_
#define _NET_BUF_H_

#include "types.h"

#define NET_BUF_SIZE        256
#define NET_BUF_DATA_SIZE   240
#define EXT_BUF_SIZE        2048

class net_buf_t {
public:
    void  init(uint8* ext_data);
    int32 append(uint8* data, uint32 len);

    uint32  m_data_len;
    uint8*  m_data;
    uint8*  m_ext_data;
    uint32  m_left;
};

#endif
