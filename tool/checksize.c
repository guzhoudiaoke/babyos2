/*
 * guzhoudiaoke@126.com
 * 2017-10-22
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SECT_SIZE 512

void hung()
{
    while (1) {
        ;
    }
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: checksize name max_sector_num\n");
        hung();
    }
    FILE *fp = fopen(argv[1], "r");

    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);
    long max_size = SECT_SIZE * atol(argv[2]);
    if (size > max_size) {
        printf("%s size %ld is larger than %ld\n", argv[1], size, max_size);
        hung();
    }

    return 0;
}
