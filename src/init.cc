/*
 * guzhoudiaoke@126.com
 * 2017-11-4
 */

#include "types.h"

static char digits[] = "0123456789abcdef";
static char buffer[64] = "This is printed by init, cs = 0x";
static char buffer2[64];
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

    __asm__ volatile("int $0x80" : : "b" (buffer), "a" (0x00));


    // fork
    int32 ret;
    __asm__ volatile("int $0x80" : "=a" (ret) : "a" (0x01));
    if (ret == 0) {
        // child
        buffer2[0] = 'I';
        buffer2[1] = 'C';
        buffer2[2] = ',';
        buffer2[3] = '\0';
        while (1) {
            for (int i = 0; i < 100000000; i++) ;
            __asm__ volatile("int $0x80" : : "b" (buffer2), "a" (0x00));
        }
    }

    buffer[0] = 'I';
    buffer[1] = ',';
    buffer[2] = '\0';
    while (1) {
        for (int i = 0; i < 100000000; i++) ;
        __asm__ volatile("int $0x80" : : "b" (buffer), "a" (0x00));
    }

    return 0;
}
