/*
 * guzhoudiaoke@126.com
 * 2017-10-22
 */

#include "types.h"
#include "kernel.h"
#include "screen.h"
#include "console.h"
#include "mm.h"

int main(void)
{
    init_screen();
    init_console();

    kprintf(WHITE, "Welcome to babyos\n");
    kprintf(RED,   "Author:\tguzhoudiaoke@126.com\n");

    init_mm();

    while (1) {
        ;
    }

    return 0;
}


