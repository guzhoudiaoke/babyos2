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

int userlib_t::exec(const char* path, argument_t* arg)
{
    int ret = 0;
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_EXEC), "b" (path), "c" (arg));
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

int userlib_t::chdir(const char* path)
{
    uint32 ret = 0;
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_CHDIR), "b" (path));
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

    int ret = fstat(fd, st);
    close(fd);
    return ret;
}

int userlib_t::pipe(int fd[2])
{
    uint32 ret = 0;
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_PIPE), "b" (fd));
    return ret;
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

int userlib_t::strncmp(const char* s1, const char *s2, int n)
{
    while (*s1 && *s2 && *s1 == *s2 && --n >= 0) {
        s1++;
        s2++;
    }

    return n == 0 ? 0 : *s1 - *s2;
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

int userlib_t::sprint_int(char* buffer, int n, int width, int base, bool sign)
{
    char buf[16] = {0};

    uint32 num = (uint32)n;
    if (sign && (sign = (n < 0))) {
        num = -n;
    }

    int i = 0;
    do {
        buf[i++] = digits[num % base];
        num /= base;
    } while (num != 0);

    if (sign) {
        buf[i++] = '-';
    }

    if (width < i) {
        width = i;
    }

    int len = i;
    while (--i >= 0) {
        *buffer++ = buf[i];
    }

    for (int i = 0; i < width - len; i++) {
        *buffer++ = ' ';
    }

    return width;
}

int userlib_t::sprint_str(char* buffer, char* s, int width)
{
    if (s == NULL) {
        return 0;
    }

    int len = strlen(s);
    if (width < len) {
        width = len;
    }

    strcat(buffer, s);
    buffer += len;
    for (int i = 0; i < width - len; i++) {
        *buffer++ = ' ';
    }
    return width;
}

bool userlib_t::is_digit(char c)
{
    return (c >= '0' && c <= '9');
}

static char str_null[] = "NULL";
int userlib_t::sprintf(char* buffer, const char *fmt, ...)
{
    buffer[0] = '\0';
    if (fmt == NULL) {
        return 0;
    }

    int total = 0;
    char sprintf_buf[BUFFER_SIZE];
    memset(sprintf_buf, 0, BUFFER_SIZE);

    va_list ap;
    va_start(ap, fmt);

    char c;
    int width = 0;
    char* s = NULL;
    for (int i = 0; (c = CHARACTER(fmt[i])) != 0; i++) {
        if (c != '%') {
            sprintf_buf[total++] = c;
            continue;
        }

        c = CHARACTER(fmt[++i]);
        if (c == '\0') {
            break;
        }

        width = 0;
        while (c != '\0' && is_digit(c)) {
            width = width * 10 + c - '0';
            c = CHARACTER(fmt[++i]);
        }

        if (c == '\0') {
            break;
        }

        switch (c) {
            case 'd':
                total += sprint_int(sprintf_buf + total, va_arg(ap, int32), width, 10, true);
                break;
            case 'u':
                total += sprint_int(sprintf_buf + total, va_arg(ap, int32), width, 10, false);
                break;
            case 'x':
            case 'p':
                total += sprint_int(sprintf_buf + total, va_arg(ap, int32), width, 16, false);
                break;
            case 'c':
                sprintf_buf[total++] = (char) CHARACTER(va_arg(ap, int32));
                break;
            case 's':
                total += sprint_str(buffer + total, va_arg(ap, char *), width);
                break;
            case '%':
                sprintf_buf[total++] = '%';
                break;
            default:
                sprintf_buf[total++] = '%';
                sprintf_buf[total++] = c;
                break;
        }
    }

    va_end(ap);
    strcpy(buffer, sprintf_buf);

    return total;
}

// only support %d %u %x %p %c %s, and seems enough for now
int userlib_t::printf(const char *fmt, ...)
{
    int total = 0;
    char buffer[BUFFER_SIZE] = {0};
    if (fmt == NULL) {
        return 0;
    }

    va_list ap;
    va_start(ap, fmt);

    char c;
    for (int i = 0; (c = CHARACTER(fmt[i])) != 0; i++) {
        if (c != '%') {
            buffer[total++] = c;
            continue;
        }

        c = CHARACTER(fmt[++i]);
        if (c == '\0') {
            break;
        }

        int width = 0;
        while (c != '\0' && is_digit(c)) {
            width = width * 10 + c - '0';
            c = CHARACTER(fmt[++i]);
        }

        if (c == '\0') {
            break;
        }

        switch (c) {
            case 'd':
                total += sprint_int(buffer + total, va_arg(ap, int32), width, 10, true);
                break;
            case 'u':
                total += sprint_int(buffer + total, va_arg(ap, int32), width, 10, false);
                break;
            case 'x':
            case 'p':
                total += sprint_int(buffer + total, va_arg(ap, int32), width, 16, false);
                break;
            case 'c':
                buffer[total++] = (char) CHARACTER(va_arg(ap, int32));
                break;
            case 's':
                total += sprint_str(buffer + total, va_arg(ap, char *), width);
                break;
            case '%':
                buffer[total++] = '%';
                break;
            default:
                buffer[total++] = '%';
                buffer[total++] = c;
                break;
        }
    }

    va_end(ap);
    print(buffer);

    return total;
}

