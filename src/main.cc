/*
 * guzhoudiaoke@126.com
 * 2017-10-22
 */

#include "babyos.h"
#include "delay.h"

babyos_t babyos;

extern "C" 
int main(void)
{
    babyos.run();
    return 0;
}

extern "C"
int apmain(uint32 index)
{
    babyos.run_ap(index);
    return 0;
}

