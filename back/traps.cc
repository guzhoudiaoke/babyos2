/*
 * guzhoudiaoke@126.com
 * 2017-10-26
 */

#include "babyos.h"
#include "x86.h"

extern "C" {

static char* exception_msg[] = {
    "int0  #DE divide error",
    "int1  #DB debug",
    "int2  --  NMI",
    "int3  #BP break point",
    "int4  #OF overflow",
    "int5  #BR bounds check",
    "int6  #UD invalid opcode",
    "int7  #NM device not available",
    "int8  #DF double fault",
    "int9  --  coprocessor seg overrun",
    "int10 #TS invalid TSS",
    "int11 #NP segment not present",
    "int12 #SS stack segment",
    "int13 #GP general protection",
    "int14 #PF page fault",
    "int15 -- （Intel reserved）",
    "int16 #MF x87 FPU coprocessor error",
    "int17 #AC align check",
};

void disp_exception_info(char* str)
{
    os()->get_console()->kprintf(RED, str);
    halt();
}

void do_divide_error()
{
    disp_exception_info(exception_msg[0]);
}

void do_debug()
{
    disp_exception_info(exception_msg[1]);
}

void do_nmi()
{
    disp_exception_info(exception_msg[2]);
}

void do_overflow()
{
    disp_exception_info(exception_msg[4]);
}

void do_bounds_check()
{
    disp_exception_info(exception_msg[5]);
}

void do_invalid_opcode()
{
    disp_exception_info(exception_msg[6]);
}

void do_device_not_available()
{
    disp_exception_info(exception_msg[7]);
}

void do_double_fault()
{
    disp_exception_info(exception_msg[8]);
}

void do_coprocessor_seg_overrun()
{
    disp_exception_info(exception_msg[9]);
}

void do_invalid_tss()
{
    disp_exception_info(exception_msg[10]);
}

void do_segment_not_present()
{
    disp_exception_info(exception_msg[11]);
}

void do_stack_segment()
{
    disp_exception_info(exception_msg[12]);
}

void do_general_protection()
{
    disp_exception_info(exception_msg[13]);
}

void do_page_fault()
{
    disp_exception_info(exception_msg[14]);
}

void do_reserved()
{
    disp_exception_info(exception_msg[15]);
}

void do_coprocessor_error()
{
    disp_exception_info(exception_msg[16]);
}

void do_alignment_check()
{
    disp_exception_info(exception_msg[17]);
}
}
