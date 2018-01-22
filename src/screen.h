/*
 * guzhoudiaoke@126.com
 * 2017-10-22
 */

#ifndef _SCREEN_H_
#define _SCREEN_H_

#include "types.h"

#define RGB(r, g, b)   ( ((uint32) (b) << 16) | ((uint32) (g)) << 8 | r )
#define RGB_GET_R(rgb) ( (uint8) (rgb) )
#define RGB_GET_G(rgb) ( (uint8) ((uint16)rgb >> 8) )
#define RGB_GET_B(rgb) ( (uint8) (rgb >> 16) )

#define RED     RGB(0xff, 0x00, 0x00)
#define GREEN   RGB(0x00, 0xff, 0x00)
#define BLUE    RGB(0x00, 0x00, 0xff)
#define WHITE   RGB(0xff, 0xff, 0xff)
#define GRAY    RGB(0xcc, 0xdd, 0xcc)
#define BLACK   RGB(0x00, 0x00, 0x00)
#define PINK    RGB(0xff, 0xc0, 0xcb)
#define PURPLE  RGB(0x80, 0x00, 0x80)
#define CYAN    RGB(0x00, 0xff, 0xff)
#define YELLOW  RGB(0xff, 0xff, 0x00)

#define ASC16_SIZE      16
#define ASC16_WIDTH     8
#define ASC16_HEIGHT    16

class screen_t {
public:
	screen_t();
	~screen_t();

	void init();
	uint32 width();
	uint32 height();
	uint8* vram();

	void set_pixel(uint32 x, uint32 y, color_ref_t color);
	void draw_asc16(char ch, uint32 left, uint32 top, color_ref_t color);
	void fill_rectangle(rect_t rect, color_ref_t color);
    void scroll();

private:
	uint8*	m_base;        // base address
	uint8*	m_asc16_addr;
	uint16	m_width;
	uint16	m_height;
	uint8	m_bytes_pp;    // bytes per pixel
};

#endif

