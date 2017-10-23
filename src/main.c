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
    init_mm();
    init_screen();
    init_console();

    kprintf(WHITE, "Welcome to babyos\n");
    kprintf(RED,   "Author:\tguzhoudiaoke@126.com\n");
    kprintf(BLUE,  "0123456789abcdef0123456789abcdef\n");
    kprintf(WHITE, "Today is %d/%d/%d, \tSunday\n", 2017, 10, 22);
    kprintf(BLUE,  "hex(%d) = %x\n", 1234, 1234);
    kprintf(GREEN, "%c - %c - %c\n", 'g', 'r', 'b');

    while (1) {
        ;
    }

    return 0;
}


