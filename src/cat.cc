/*
 * guzhoudiaoke@126.com
 * 2018-01-13
 */

#include "userlib.h"

void cat(int fd)
{
    char buf[512];

    int n = 0;
    while ((n = userlib_t::read(fd, buf, sizeof(buf))) > 0) {
        if (userlib_t::write(1, buf, n) != n) {
            userlib_t::printf("cat: write error\n");
            userlib_t::exit(-1);
        }
    }
}

int main(int argc, char** argv)
{
    if (argc <= 1) {
        cat(0);
        userlib_t::exit(0);
    }

    for (int i = 1; i < argc; i++) {
        int fd = userlib_t::open(argv[i], file_t::MODE_RDONLY);
        if (fd < 0) {
            userlib_t::printf("cat: cannot open %s\n", argv[i]);
            userlib_t::exit(0);
        }

        cat(fd);
        userlib_t::close(fd);
    }

    userlib_t::exit(0);
}

