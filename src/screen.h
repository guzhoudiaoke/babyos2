/*
 * guzhoudiaoke@126.com
 * 2017-10-22
 */

#ifndef _SCREEN_H_
#define _SCREEN_H_

#include "types.h"

typedef struct video_info_s {
    uint16 video_mode;
    uint16 width;
    uint16 height;
    uint8  bits_per_pixel;
    uint8  memory_model;
    uint8* vram_base_addr;
} video_info_t;


typedef struct screen_s {
    uint8* base;        // base address
    uint8* asc16_addr;
    uint16 width;
    uint16 height;
    uint8  bytes_pp;    // bytes per pixel
} screen_t;

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

#define RGB(r, g, b)   ( ((uint32) (b) << 16) | ((uint32) (g)) << 8 | r )
#define RGB_GET_R(rgb) ( (uint8) (rgb) )
#define RGB_GET_G(rgb) ( (uint8) ((uint16)rgb >> 8) )
#define RGB_GET_B(rgb) ( (uint8) (rgb >> 16) )

#define RED     RGB(0xff, 0x00, 0x00)
#define GREEN   RGB(0x00, 0xff, 0x00)
#define BLUE    RGB(0x00, 0x00, 0xff)
#define WHITE   RGB(0xff, 0xff, 0xff)
#define BLACK   RGB(0x00, 0x00, 0x00)

#define ASC16_SIZE      16
#define ASC16_WIDTH     8
#define ASC16_HEIGHT    16

int32  init_screen();
uint32 screen_width();
uint32 screen_height();

void   set_pixel(uint32 x, uint32 y, color_ref_t color);
void   draw_asc16(char ch, uint32 left, uint32 top, color_ref_t color);
void   fill_rectangle(rect_t rect, color_ref_t color);

#endif

