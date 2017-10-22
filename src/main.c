/*
 * guzhoudiaoke@126.com
 * 2017-10-22
 */

#include "types.h"
#include "kernel.h"

typedef struct vidoe_info_s {
    uint16 video_mode;
    uint16 cx_screen;
    uint16 cy_screen;
    uint8  n_bits_per_pixel;
    uint8  n_memory_model;
    uint8* p_vram_base_addr;
} video_info_t;

video_info_t* p_video_info = (video_info_t *)VIDEO_INFO_ADDR;

uint8* p_vram_base_addr = (uint8 *)0xe0000000;
uint32 cx_screen = 1024;
uint32 cy_screen = 768;
uint32 n_bytes_per_pixel = 3;

void set_pixel(int32 x, int32 y, uint8 r, uint8 g, uint8 b)
{
    uint8* pvram = NULL;

    pvram = p_vram_base_addr + n_bytes_per_pixel*y*cx_screen + n_bytes_per_pixel*x;
    pvram[0] = b;
    pvram[1] = g;
    pvram[2] = r;
}

int main(void)
{
    p_vram_base_addr = p_video_info->p_vram_base_addr;

    for (int i = 0x100; i < 0x120; i++) {
        set_pixel(i, 500, 0x00, 0x00, 0xff);
    }

    while (1) {
        ;
    }

    return 0;
}

