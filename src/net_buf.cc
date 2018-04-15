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

int32 net_buf_t::append(const void* data, uint32 len)
{
    if (m_left < len) {
        return -ENOBUFS;
    }

    memcpy(m_data + m_data_len, data, len);
    m_data_len += len;
    m_left -= len;

    return 0;
}

int32 net_buf_t::append_zero(uint32 len)
{
    if (m_left < len) {
        return -ENOBUFS;
    }

    memset(m_data + m_data_len, 0, len);
    m_data_len += len;
    m_left -= len;

    return 0;
}

uint8* net_buf_t::get_data()
{
    return m_data;
}

uint32 net_buf_t::get_data_len()
{
    return m_data_len;
}

uint8* net_buf_t::get_ext_data()
{
    return m_ext_data;
}

int32 net_buf_t::pop_front(uint32 len)
{
    if (m_data_len < len) {
        return -EINVAL;
    }

    m_data += len;
    m_data_len -= len;

    return 0;
}
