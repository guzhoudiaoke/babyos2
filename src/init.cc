/*
 * guzhoudiaoke@126.com
 * 2017-11-4
 */

#include "userlib.h"

#define SHELL_LBA       544
#define SHELL_SECT_NUM  32

int main()
{
    uint32 cs = 0xffffffff;
    __asm__ volatile("movl %%cs, %%eax" : "=a" (cs));

    // print cs to show work in user mode
    userlib_t::print("This is printed by init, cs = ");
    userlib_t::print_int(cs, 16, 0);
    userlib_t::print("\n");

    // test mmap
    //unsigned* before_fork = (unsigned *) userlib_t::mmap(0, 4096, PROT_READ | PROT_WRITE, 0);
    //for (unsigned int i = 0; i < 1024; i++) {
    //    before_fork[i] = i;
    //}

    // fork
    int32 ret = userlib_t::fork();
    if (ret == 0) {
        // child
        int ret = userlib_t::exec(SHELL_LBA, SHELL_SECT_NUM);
        if (ret != 0) {
            userlib_t::print("exec failed!!!\n");
        }
        while (1) {
            for (int i = 0; i < 100000000; i++) ;
            userlib_t::print("IC,");
        }
    }
    else {
        // parent
        while (1) {
            for (int i = 0; i < 100000000; i++) ;
            userlib_t::print("I,");
        }
    }

    return 0;
}

