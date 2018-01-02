/*
 * guzhoudiaoke@126.com
 * 2017-12-10
 */

#include "shell.h"
#include "userlib.h"

void process_signal(int32 sig)
{
    userlib_t::print("SIG_");
    userlib_t::print_int(sig, 10, 1);
    userlib_t::print(",");
}

int main()
{
    sighandler_t handler = &process_signal;
    userlib_t::signal(4, handler);
    userlib_t::print("This is printed by shell.\n");

    int times = 0;
    while (times++ < 5) {
        userlib_t::sleep(2);
        userlib_t::print("S,");
    }

    userlib_t::exit(0);

    int* p = (int *) 0xa0000000;
    *p = 0x1234;

    while (1) {
        userlib_t::sleep(2);
        userlib_t::print("s,");
    }

    userlib_t::exit(0);
    return 1;
}

