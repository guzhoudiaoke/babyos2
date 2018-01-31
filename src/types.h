/*
 * guzhoudiaoke@126.com
 * 2017-10-21
 */

#ifndef _TYPES_H_
#define _TYPES_H_

#include "errno.h"

#define NULL				(0)

typedef unsigned char       uint8;
typedef unsigned short      uint16;
typedef unsigned int        uint32;
typedef unsigned long long  uint64;
typedef char                int8;
typedef short               int16;
typedef int                 int32;
typedef long long           int64;

typedef int32				pid_t;

/* video info */
typedef struct video_info_s {
    uint16 video_mode;
    uint16 width;
    uint16 height;
    uint8  bits_per_pixel;
    uint8  memory_model;
    uint8* vram_base_addr;
} video_info_t;

typedef struct address_range_s {
	uint32	base_addr_low;
	uint32	base_addr_high;
	uint32	length_low;
	uint32	lenght_high;
    uint32  type;
} address_range_t;

typedef struct memory_layout_s {
	uint32 num_of_range;
	address_range_t ranges[32];
} memory_layout_t;

typedef struct rgb_s {
    uint8 r;
    uint8 g;
    uint8 b;
} rgb_t;

typedef struct rect_s {
    int32 left;
    int32 top;
    uint32 width;
    uint32 height;
} rect_t;

typedef uint32 color_ref_t;

typedef uint32  pde_t;
typedef uint32  pte_t;


#define MAX_ARGS 16
#define MAX_ARG_LEN 32

typedef struct argument_s {
    unsigned m_argc;
    char m_argv[MAX_ARGS][MAX_ARG_LEN];
} argument_t;


#endif
