/*
 * guzhoudiaoke@126.com
 * 2018-04-08
 */

#include "net_buf.h"
#include "string.h"

void net_buf_t::init(uint8* ext_data)
{
    m_data_len = 0;
    if (ext_data != NULL) {
        m_ext_data = ext_data;
        m_data = ext_data;
        m_left = EXT_BUF_SIZE;
    }
    else {
        m_ext_data = NULL;
        m_data = ((uint8 *) this) + sizeof(net_buf_t);
        m_left = NET_BUF_DATA_SIZE;
    }
}

int32 net_buf_t::append(uint8* data, uint32 len)
{
    if (m_left < len) {
        return -ENOBUFS;
    }

    memcpy(m_data + m_data_len, data, len);
    m_data_len += len;
    m_left -= len;

    return 0;
}

