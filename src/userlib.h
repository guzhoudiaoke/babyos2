/*
 * guzhoudiaoke@126.com
 * 2017-12-10
 */

#ifndef _USERLIB_H_
#define _USERLIB_H_

#include "types.h"
#include "syscall.h"
#include "signal.h"
#include "fs.h"

#define PROT_NONE           0x0       /* page can not be accessed */
#define PROT_READ           0x1       /* page can be read */
#define PROT_WRITE          0x2       /* page can be written */
#define PROT_EXEC           0x4       /* page can be executed */


class userlib_t {
public:
    static int fork();
    //static int exec(uint32 lba, uint32 sector_num, const char* name);
    static int exec(const char* path);
    static void *mmap(uint32 addr, uint32 len, uint32 prot, uint32 flags);
    static void exit(int val);
    static void wait(uint32 pid);
    static void kill(uint32 pid, uint32 sig);
    static void signal(uint32 sig, sighandler_t handler);

    static int  print(const char *str);
    static void print_int(int32 n, int32 base, int32 sign);

    static void loop_delay(int32 loop);
    static void sleep(uint32 second);

    static char* strrev(char* str, int len);
    static void* memset(void* dst, uint32 c, uint32 n);
    static int   strlen(const char* str);
    static char* strcpy(char* dst, const char* src);
    static char* strncpy(char* dst, const char* src, int n);
    static int   strcmp(const char* s1, const char *s2);
    static char* strcat(char* dst, const char* src);

    static int  open(const char* path, int mode);
    static int  close(int fd);
    static int  read(int fd, void* buf, uint32 size);
    static int  write(int fd, void* buf, uint32 size);
    static int  mkdir(const char* path);
    static int  link(const char* path_old, const char* path_new);
    static int  unlink(const char* path);
    static int  mknod(const char* path, int major, int minor);
    static int  dup(int fd);

    static int  fstat(int fd, stat_t* st);
    static int  stat(const char* path, stat_t* st);
};

#endif
