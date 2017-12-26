/*
 * guzhoudiaoke@126.com
 * 2017-12-10
 */

#include "shell.h"
#include "userlib.h"

int main()
{
    userlib_t::print("This is printed by shell.\n");

    int times = 0;
    while (++times < 5) {
        userlib_t::sleep(2);
        userlib_t::print("S,");
    }

    userlib_t::exit(0);

    return 1;
}

