/*
 * guzhoudiaoke@126.com
 * 2018-01-03
 */

#include "bitmap.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void bitmap_t::init(unsigned int num)
{
    m_num = (num + 4095) & (~4095);
    int size = m_num / 8;
    m_set = (unsigned char*) malloc(sizeof(unsigned char) * (m_num/8));
    memset(m_set, 0, size);
}

bool bitmap_t::testbit(unsigned int index)
{
    if (index >= m_num) {
        return false;
    }

    unsigned char* p = m_set;
    p += index >> 3;
    return (*p & (1 << (7 - (index % 8))));
}

void bitmap_t::setbit(unsigned int index)
{
    if (index >= m_num) {
        return;
    }
    unsigned char* p = m_set;
    p += index >> 3;
    *p |= 1 << (7 - (index % 8));
}

void bitmap_t::unsetbit(unsigned int index)
{
    if (index >= m_num) {
        return;
    }
    unsigned char* p = m_set;
    p += index >> 3;
    *p &= ~(1 << (7 - (index % 8)));
}

void bitmap_t::clear()
{
    memset(m_set, 0, sizeof(m_set));
}

int bitmap_t::getnext()
{
    for (unsigned int i = 0; i < m_num; ++i) {
        if (!testbit(i)) {
            return i;
        }
    }
    return -1;
}

void bitmap_t::setbits(unsigned char* buffer, unsigned index, unsigned count)
{
    memcpy(m_set + index/8, buffer, count/8);

    buffer += index/8;
    for (int i = 0; i < count % 8; i++) {
        if (*buffer & (1 << (7 - i))) {
            setbit(index + i);
        }
        else {
            unsetbit(index + i);
        }
    }
}

void bitmap_t::dump()
{
    printf("m_size: %u\n", m_num);
    //for (int i = 0; i < m_num; i++) {
    //    if (testbit(i)) {
    //        printf("1 ");
    //    }
    //    else {
    //        printf("0 ");
    //    }
    //}
    unsigned *u = (unsigned *) m_set;
    for (int i = 0; i < m_num / 8 / 4; i++) {
        printf("%x, ", *u);
        u++;
    }

    printf("\n");
}

