/*
 * guzhoudiaoke@126.com
 * 2017-12-10
 */

#include "userlib.h"

static char digits[] = "0123456789abcdef";

int userlib_t::fork()
{
    int ret;
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_FORK));
    return ret;
}

int userlib_t::exec(uint32 lba, uint32 sector_num, const char* name)
{
    int ret = 0;
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_EXEC), "b" (lba), "c" (sector_num), "d" (name));
    return ret;
}

void userlib_t::print(const char *str)
{
    int ret = 0;
    __asm__ volatile("int $0x80" : "=a" (ret) : "b" (str), "a" (SYS_PRINT));
}

void *userlib_t::mmap(uint32 addr, uint32 len, uint32 prot, uint32 flags)
{
    uint32 ret = 0;
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_MMAP), "b" (addr), "c" (len), "d" (prot), "S" (flags));

    return (void*) ret;
}

void userlib_t::exit(int val)
{
    __asm__ volatile("int $0x80" : : "b" (val), "a" (SYS_EXIT));
}

void userlib_t::wait(uint32 pid)
{
    __asm__ volatile("int $0x80" : : "b" (pid), "a" (SYS_WAIT));
}

char* userlib_t::strrev(char* str, int len)
{
    char* s = str;
    char* e = str + len -1;
    while (s < e) {
        char c = *s;
        *s++ = *e;
        *e-- = c;
    }

    return str;
}

void userlib_t::print_int(int32 n, int32 base, int32 sign)
{
    char buffer[16] = {0};

    uint32 num = (uint32)n;
    if (sign && (sign = (n < 0))) {
        num = -n;
    }

    int i = 0;
    do {
        buffer[i++] = digits[num % base];
        num /= base;
    } while (num != 0);

    if (base == 16) {
        while (i < 8) {
            buffer[i++] = '0';
        }
        buffer[i++] = 'x';
        buffer[i++] = '0';
    }

    if (sign) {
        buffer[i++] = '-';
    }

    strrev(buffer, i);
    print(buffer);
}

void userlib_t::loop_delay(int32 loop)
{
    while (--loop > 0) {
        __asm__("nop");
    }
}

void userlib_t::sleep(uint32 second)
{
    __asm__ volatile("int $0x80" : : "b" (second), "a" (SYS_SLEEP));
}

void userlib_t::kill(uint32 pid, uint32 sig)
{
    __asm__ volatile("int $0x80" : : "b" (pid), "c"(sig), "a" (SYS_KILL));
}

void userlib_t::signal(uint32 sig, sighandler_t handler)
{
    __asm__ volatile("int $0x80" : : "b" (sig), "c"(handler), "a" (SYS_SIGNAL));
}


// file
int userlib_t::open(const char* path, int mode)
{
    uint32 ret = 0;
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_OPEN), "b" (path), "c" (mode));
    return ret;
}

int userlib_t::close(int fd)
{
    uint32 ret = 0;
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_CLOSE), "b" (fd));
    return ret;
}

int userlib_t::read(int fd, char* buf, uint32 size)
{
    uint32 ret = 0;
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_READ), "b" (fd), "c" ((uint32) buf), "d" (size));
    return ret;
}

int userlib_t::write(int fd, char* buf, uint32 size)
{
    uint32 ret = 0;
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_WRITE), "b" (fd), "c" ((uint32) buf), "d" (size));
    return ret;
}

int userlib_t::mkdir(const char* path)
{
    return 0;
}

int userlib_t::link(const char* path_old, const char* path_new)
{
    uint32 ret = 0;
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_LINK), "b" ((uint32) path_old), "c" ((uint32) path_new));
    return ret;
}

int userlib_t::unlink(const char* path)
{
    uint32 ret = 0;
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_UNLINK), "b" (path));
    return ret;
}

