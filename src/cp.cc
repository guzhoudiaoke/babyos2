/*
 * guzhoudiaoke@126.com
 * 2018-01-13
 */

#include "userlib.h"

int main(int argc, char** argv)
{
    if (argc < 3) {
        userlib_t::printf("Usage: cp from to \n");
        userlib_t::exit(0);
    }

    int fd = userlib_t::open(argv[1], file_t::MODE_RDONLY);
    if (fd < 0) {
        userlib_t::printf("can't open %s\n", argv[1]);
        userlib_t::exit(0);
    }

    int fd2 = userlib_t::open(argv[2], file_t::MODE_CREATE | file_t::MODE_WRONLY);
    if (fd2 < 0) {
        userlib_t::printf("can't create %s\n", argv[2]);
        userlib_t::exit(0);
    }

    char buf[512];

    int n = 0;
    while ((n = userlib_t::read(fd, buf, sizeof(buf))) > 0) {
        if (userlib_t::write(fd2, buf, n) != n) {
            userlib_t::printf("cat: write error\n");
            userlib_t::exit(-1);
        }
    }

    userlib_t::close(fd);
    userlib_t::close(fd2);

    userlib_t::exit(0);
}

