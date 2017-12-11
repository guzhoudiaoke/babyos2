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

int userlib_t::exec(uint32 lba, uint32 sector_num)
{
    int ret = 0;
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_EXEC), "b" (lba), "c" (sector_num));
    return ret;
}

void userlib_t::print(const char *str)
{
    __asm__ volatile("int $0x80" : : "b" (str), "a" (SYS_PRINT));
}

void *userlib_t::mmap(uint32 addr, uint32 len, uint32 prot, uint32 flags)
{
    uint32 ret = 0;
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_MMAP), "b" (addr), "c" (len), "d" (prot), "S" (flags));

    return (void*) ret;
}

char buffer_int[16] = {1};
char buffer_int_r[16] = {1};
void userlib_t::print_int(int32 n, int32 base, int32 sign)
{

    uint32 num = (uint32)n;
    if (sign && (sign = (n < 0))) {
        num = -n;
    }

    int i = 0;
    do {
        buffer_int[i++] = digits[num % base];
        num /= base;
    } while (num != 0);

    if (base == 16) {
        while (i < 8) {
            buffer_int[i++] = '0';
        }
        buffer_int[i++] = 'x';
        buffer_int[i++] = '0';
    }

    if (sign) {
        buffer_int[i++] = '-';
    }

    int j = 0;
    while (i-- > 0) {
        buffer_int_r[j++] = buffer_int[i];
    }

    buffer_int_r[j++] = ',';
    print(buffer_int_r);
}

