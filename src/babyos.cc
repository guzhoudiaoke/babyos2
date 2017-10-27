/*
 * guzhoudiaoke@126.com
 * 2017-10-26
 */

#include "babyos.h"

extern BabyOS os;
BabyOS *BabyOS::get_os()
{
    return &os;
}

BabyOS::BabyOS()
{
}

BabyOS::~BabyOS()
{
}

Screen *BabyOS::get_screen() 
{
    return &m_screen; 
}

Console *BabyOS::get_console()
{
    return &m_console;
}

MM* BabyOS::get_mm()
{
    return &m_mm;
}

Arch* BabyOS::get_arch()
{
    return &m_arch;
}

void BabyOS::run()
{
    m_screen.init();
    m_console.init();

    m_console.kprintf(WHITE, "Welcome to babyos\n");
    m_console.kprintf(RED,   "Author:\tguzhoudiaoke@126.com\n");

    m_mm.init();
    m_arch.init();

    while (1) {
    }
}
