/*
 * guzhoudiaoke@126.com
 * 2017-12-10
 */

#ifndef _USERLIB_H_
#define _USERLIB_H_

#include "types.h"
#include "syscall.h"


#define PROT_NONE           0x0       /* page can not be accessed */
#define PROT_READ           0x1       /* page can be read */
#define PROT_WRITE          0x2       /* page can be written */
#define PROT_EXEC           0x4       /* page can be executed */


class userlib_t {
public:
    static int fork();
    static int exec(uint32 lba, uint32 sector_num, const char* name);
    static void *mmap(uint32 addr, uint32 len, uint32 prot, uint32 flags);
    static void exit(int val);

    static void print(const char *str);
    static void print_int(int32 n, int32 base, int32 sign);

    static void loop_delay(uint32 loop);
};

#endif
