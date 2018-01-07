/*
 * guzhoudiaoke@126.com
 * 2017-10-22
 */

#include "types.h"
#include "babyos.h"
#include "kernel.h"
#include "screen.h"
#include "string.h"
#include "mm.h"

screen_t::screen_t()
{
}

screen_t::~screen_t()
{
}

void screen_t::init()
{
    video_info_t *info = (video_info_t *) PA2VA(VIDEO_INFO_ADDR);
    m_width    = info->width; 
    m_height   = info->height; 
    m_bytes_pp = info->bits_per_pixel / 8;
    m_base     = (info->vram_base_addr);
    m_asc16_addr = (uint8 *) PA2VA(FONT_ASC16_ADDR);
}

uint32 screen_t::width()
{
    return m_width;
}

uint32 screen_t::height()
{
    return m_height;
}

uint8* screen_t::vram()
{
    return m_base;
}

void screen_t::set_pixel(uint32 x, uint32 y, color_ref_t color)
{
    if (x < m_width && y < m_height) {
        uint8* pvram = m_base + m_bytes_pp*y*m_width + m_bytes_pp*x;
        pvram[0] = RGB_GET_B(color);
        pvram[1] = RGB_GET_G(color);
        pvram[2] = RGB_GET_R(color);
    }
}

void screen_t::draw_asc16(char ch, uint32 left, uint32 top, color_ref_t color)
{
    uint8* p_asc = m_asc16_addr + ch * ASC16_SIZE;

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

void screen_t::fill_rectangle(rect_t rect, color_ref_t color)
{
    for (uint32 y = 0; y < rect.height; ++y)
    {
        for (uint32 x = 0; x < rect.width; ++x) {
            set_pixel(rect.left + x, rect.top + y, color);
        }
    }
}

void screen_t::scroll()
{
    memmov(m_base, m_base + m_width*m_bytes_pp*ASC16_HEIGHT, m_width*m_bytes_pp*(m_height-ASC16_HEIGHT));
}

