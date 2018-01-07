/*
 * guzhoudiaoke@126.com
 * 2017-12-10
 */

#include "shell.h"
#include "userlib.h"
#include "file.h"

int main()
{
    int fd = userlib_t::open("/test", file_t::MODE_RDWR);
    if (fd < 0) {
        userlib_t::print("failed to open file abc\n");
        userlib_t::exit(-1);
    }

    char buf[512] = {0};
    int n = userlib_t::read(fd, buf, 512);
    userlib_t::print_int(n, 10, 1);
    userlib_t::print(" bytes read.\n");
    userlib_t::print(buf);

    userlib_t::close(fd);
    userlib_t::exit(0);
    return 1;
}

#if 0
void process_signal(int32 sig)
{
    userlib_t::print("SIG_");
    userlib_t::print_int(sig, 10, 1);
    userlib_t::print(",");
}

int main()
{
    userlib_t::exit(0);
    return 1;

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
#endif

