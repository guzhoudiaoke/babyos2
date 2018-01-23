/*
 * guzhoudiaoke@126.com
 * 2018-01-23
 */

#ifndef _MATH_H_
#define _MATH_H_

#include "types.h"

class math_t {
public:
    static uint32 min(uint32 a, uint32 b);
    static uint32 max(uint32 a, uint32 b);
    static uint32 log(int32 x, int32 n);
    static uint32 pow(int32 x, int32 p);
};

#endif
