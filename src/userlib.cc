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

int userlib_t::exec(const char* path)
{
    int ret = 0;
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_EXEC), "b" (path));
    return ret;
}

int userlib_t::print(const char *str)
{
    int ret = 0;
    __asm__ volatile("int $0x80" : "=a" (ret) : "b" (str), "a" (SYS_PRINT));
    return ret;
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

int userlib_t::read(int fd, void* buf, uint32 size)
{
    uint32 ret = 0;
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_READ), "b" (fd), "c" ((uint32) buf), "d" (size));
    return ret;
}

int userlib_t::write(int fd, void* buf, uint32 size)
{
    uint32 ret = 0;
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_WRITE), "b" (fd), "c" ((uint32) buf), "d" (size));
    return ret;
}

int userlib_t::mkdir(const char* path)
{
    uint32 ret = 0;
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_MKDIR), "b" (path));
    return ret;
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

int userlib_t::mknod(const char* path, int major, int minor)
{
    uint32 ret = 0;
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_MKNOD), "b" (path), "c" (major), "d" (minor));
    return ret;
}

int userlib_t::dup(int fd)
{
    uint32 ret = 0;
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_DUP), "b" (fd));
    return ret;
}

int userlib_t::fstat(int fd, stat_t* st)
{
    uint32 ret = 0;
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_STAT), "b" (fd), "c" (st));
    return ret;
}

int userlib_t::stat(const char* path, stat_t* st)
{
    int fd = open(path, file_t::MODE_RDONLY);
    if (fd < 0) {
        return -1;
    }

    return fstat(fd, st);
}

void* userlib_t::memset(void *dst, uint32 c, uint32 n)
{
    char* d = (char *) dst;
    for (uint32 i = 0; i < n; i++) {
        *d++ = (c & 0xff);
    }

    return dst;
}

int userlib_t::strlen(const char* s)
{
    int len = 0;
    while (*s++) {
        len++;
    }
    return len;
}


char* userlib_t::strcpy(char* dst, const char* src)
{
	char* d = dst;
	while (*src) {
		*d++ = *src++;
	}
    *d++ = '\0';

	return dst;
}

char* userlib_t::strncpy(char* dst, const char* src, int n)
{
	char* d = dst;
	while (--n >= 0 && *src) {
		*d++ = *src++;
	}
    *d++ = '\0';

	return dst;
}

int userlib_t::strcmp(const char* s1, const char *s2)
{
    while (*s1 && *s2 && *s1 == *s2) {
        s1++;
        s2++;
    }

    return *s1 - *s2;
}

char* userlib_t::strcat(char* dst, const char* src)
{
    char* ret = dst;
    while (*dst) {
        dst++;
    }
    while (*src) {
        *dst++ = *src++;
    }
    return ret;
}

