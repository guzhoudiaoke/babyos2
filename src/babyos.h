/*
 * guzhoudiaoke@126.com
 * 2017-10-26
 */

#ifndef _BABYOS_H_
#define _BABYOS_H_

#include "screen.h"
#include "console.h"
#include "mm.h"
#include "arch.h"

class BabyOS {
public:
    BabyOS();
    ~BabyOS();

    void run();

    Screen *get_screen();
    Console *get_console();
    MM *get_mm();
    Arch *get_arch();

    static BabyOS *get_os();

private:
    Screen	m_screen;
    Console m_console;
    MM		m_mm;
    Arch    m_arch;
};

#define os()	BabyOS::get_os()

#endif
