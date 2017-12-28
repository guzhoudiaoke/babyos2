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

    while (1) {
        userlib_t::sleep(2);
        userlib_t::print("S,");
    }

    userlib_t::exit(0);
    return 1;
}

