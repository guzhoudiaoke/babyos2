/*
 * guzhoudiaoke@126.com
 * 2017-12-10
 */

#include "shell.h"
#include "userlib.h"

int main()
{
    userlib_t::print("This is printed by shell.\n");

    int time = 10;
    while (time--) {
        for (int i = 0; i < 100000000; i++) {
        }
        userlib_t::loop_delay(100000000);
        userlib_t::print("S,");
    }

    userlib_t::exit(0);
    return 1;
}

