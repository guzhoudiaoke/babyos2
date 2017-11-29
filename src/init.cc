/*
 * guzhoudiaoke@126.com
 * 2017-11-4
 */

#include "types.h"
#include "syscall.h"

#define PROT_NONE           0x0       /* page can not be accessed */
#define PROT_READ           0x1       /* page can be read */
#define PROT_WRITE          0x2       /* page can be written */
#define PROT_EXEC           0x4       /* page can be executed */

static char digits[] = "0123456789abcdef";
static char buffer[64] = "This is printed by init, cs = 0x";
static char buffer2[64];
static unsigned times = 0;
static unsigned times2 = 0;
static char test[16] = "PARENT\n";
static char test2[16] = "CHILD\n";
static unsigned* before_fork;
static unsigned* p;

inline int fork()
{
    int ret;
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_FORK));
    return ret;
}

inline void print(char *str)
{
    __asm__ volatile("int $0x80" : : "b" (str), "a" (SYS_PRINT));
}

inline void *mmap(uint32 addr, uint32 len, uint32 prot, uint32 flags)
{
    uint32 ret = 0;
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (SYS_MMAP), "b" (addr), "c" (len), "d" (prot), "S" (flags));

    return (void*) ret;
}

inline void test_seg_fault()
{
    for (unsigned i = 0; i < 1024; i++) {
        p[i] = i;
    }
}

inline void test_fault()
{
    p = (unsigned *) mmap(0, 4096, 0, 0);
    test_seg_fault();
}


char buffer_int[16] = {0};
char buffer_int_r[16] = {0};
inline void print_int(int32 n, int32 base, int32 sign)
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


int main()
{
    uint32 cs = 0xffffffff;
    __asm__ volatile("movl %%cs, %%eax" : "=a" (cs));

    int i = 0;
    while (buffer[i] != '\0') {
        i++;
    }

    char num[9] = {0};
    int j = 0;
    while (cs != 0) {
        num[j++] = digits[cs % 16];
        cs /= 16;
    }

    while (j) {
        buffer[i++] = num[--j];
    }
    buffer[i++] = '\n';
    print(buffer);

    before_fork = (unsigned *) mmap(0, 4096, PROT_READ | PROT_WRITE, 0);
    for (unsigned int i = 0; i < 1024; i++) {
        before_fork[i] = i;
    }

    // fork
    int32 ret = fork();
    if (ret == 0) {
        // child
        buffer2[0] = 'I';
        buffer2[1] = 'C';
        buffer2[2] = ',';
        buffer2[3] = '\0';
        while (1) {
            for (int i = 0; i < 100000000; i++) ;
            print(buffer2);

            times2++;
            if (times2 == 10) {
                print(test2);
                for (unsigned int i = 100; i < 110; i++) {
                    before_fork[i] = i+1;
                }
                for (unsigned int i = 0; i < 16; i++) {
                    before_fork[i] = 100*i;
                    print_int(before_fork[i], 16, 0);
                }
            }
        }
    }
    else {
        // parent
        buffer[0] = 'I';
        buffer[1] = ',';
        buffer[2] = '\0';

        while (1) {
            for (int i = 0; i < 100000000; i++) ;
            print(buffer);

            times++;
            if (times == 15) {
                print(test);
                for (unsigned int i = 200; i < 210; i++) {
                    before_fork[i] = i;
                }
                for (unsigned int i = 0; i < 16; i++) {
                    print_int(before_fork[i], 16, 0);
                }
            }
        }
    }

    return 0;
}

