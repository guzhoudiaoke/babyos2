/*
 * guzhoudiaoke@126.com
 * 2018-01-23
 */

#include "math.h"

uint32 math_t::log(int32 x, int32 n)
{
    int32 ret = 0, num = 1;
    while (num < n) {
        num *= x;
        ret++;
    }

    return ret;
}

uint32 math_t::min(uint32 a, uint32 b)
{
    return a < b ? a : b;
}

uint32 math_t::max(uint32 a, uint32 b)
{
    return a > b ? a : b;
}

uint32 math_t::pow(int32 x, int32 p)
{
    uint32 ret = 1;
    for (uint32 i = 0; i < p; i++) {
        ret *= x;
    }
    return ret;
}
