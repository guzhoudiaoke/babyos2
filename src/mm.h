/*
 * guzhoudiaoke@126.com
 * 2017-10-23
 */

#ifndef _MM_H_
#define _MM_H_

#include "types.h"

typedef uint32  pde_t;
typedef uint32  pte_t;

void init_mm();
void kmap_device(void *va);

#endif
