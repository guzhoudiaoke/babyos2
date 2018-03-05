/*
 * guzhoudiaoke@126.com
 * 2018-03-05
 */

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

