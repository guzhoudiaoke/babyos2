/*
 * guzhoudiaoke@126.com
 * 2018-01-13
 */

#include "userlib.h"

int main(int argc, char** argv)
{
    if (argc < 3) {
        userlib_t::printf("Usage: ln file_from file_to\n");
        userlib_t::exit(0);
    }

    if (userlib_t::link(argv[1], argv[2]) < 0) {
        userlib_t::printf("link %s %s failed\n", argv[1], argv[2]);
    }

    userlib_t::exit(0);
}


