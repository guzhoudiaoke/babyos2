/*
 * guzhoudiaoke@126.com
 * 2018-01-03
 */

#ifndef _BITMAP_H_
#define _BITMAP_H_

class bitmap_t
{
public:
    void init(unsigned int num);
    bool testbit(unsigned int index);
    void setbit(unsigned int index);
    void unsetbit(unsigned int index);
    void clear();
    int  getnext();
    void setbits(unsigned char* buffer, unsigned index, unsigned count);
    void dump();
    unsigned char* buffer() { return m_set; }

private:
    unsigned int    m_num;
    unsigned char*  m_set;
};

#endif
