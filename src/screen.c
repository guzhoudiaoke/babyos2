/*
 * guzhoudiaoke@126.com
 * 2017-10-22
 */

#include "types.h"
#include "kernel.h"
#include "screen.h"
#include "string.h"
#include "mm.h"

static screen_t screen;

int32 init_screen()
{
    video_info_t *info = (video_info_t *) PA2VA(VIDEO_INFO_ADDR);
    screen.width    = info->width; 
    screen.height   = info->height; 
    screen.bytes_pp = info->bits_per_pixel / 8;
    screen.base     = (info->vram_base_addr);

    screen.asc16_addr = (uint8 *) PA2VA(FONT_ASC16_ADDR);

    kmap_device(screen.base);

    return 0;
}

uint32 screen_width()
{
    return screen.width;
}

uint32 screen_height()
{
    return screen.height;
}

void set_pixel(uint32 x, uint32 y, color_ref_t color)
{
    if (x < screen.width && y < screen.height) {
        uint8* pvram = screen.base + screen.bytes_pp*y*screen.width + screen.bytes_pp*x;
        pvram[0] = RGB_GET_B(color);
        pvram[1] = RGB_GET_G(color);
        pvram[2] = RGB_GET_R(color);
    }
}

void draw_asc16(char ch, uint32 left, uint32 top, color_ref_t color)
{
    uint8* p_asc = screen.asc16_addr + ch * ASC16_SIZE;

	for (int32 y = 0; y < ASC16_HEIGHT; y++) {
		uint8 test_bit = 1 << 7;
		for (int32 x = 0; x < ASC16_WIDTH; x++) {
			if (*p_asc & test_bit) {
				set_pixel(left+x, top+y, color);
            }

			test_bit >>= 1;
		}
		p_asc++;
	}
}

void fill_rectangle(rect_t rect, color_ref_t color)
{
    for (uint32 y = 0; y < rect.height; ++y)
    {
        for (uint32 x = 0; x < rect.width; ++x) {
            set_pixel(rect.left + x, rect.top + y, color);
        }
    }
}

