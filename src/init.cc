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

    // fork
    int32 ret = userlib_t::fork();
    if (ret == 0) {
        // child
        ret = userlib_t::exec(SHELL_LBA, SHELL_SECT_NUM, "shell");
        if (ret != 0) {
            userlib_t::print("BUG exec failed!!!\n");
        }
        while (1) {
            userlib_t::loop_delay(100000000);
            userlib_t::print("IC,");
        }
    }
    else {
        // parent
        while (1) {
            userlib_t::loop_delay(100000000);
            userlib_t::print("I,");
        }
    }

    return 0;
}

