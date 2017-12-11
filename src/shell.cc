/*
 * guzhoudiaoke@126.com
 * 2017-12-10
 */

#include "shell.h"
#include "userlib.h"

int main()
{
    userlib_t::print("This is printed by shell...\n");
    while (1) {
        for (int i = 0; i < 100000000; i++) ;
        userlib_t::print("S,");
    }

    return 0;
}
