/*
 * guzhoudiaoke@126.com
 * 2017-10-27
 */

#include "kernel.h"

# ----------------------------------------------------------------------------------------------------------------------
# 向量号  助记符 说明                        类型        错误号    产生源
# --------------------------------------------------------------------------------------------------------------------
# 0       #DE    除出错                      故障        无        DIV或IDIV指令
# 1       #DB    调试                        故障/陷阱   无        任何代码或数据引用，或是INT 1指令
# 2        --    NMI中断                     中断        无        非屏蔽外部中断
# 3       #BP    断点                        陷阱        无        INT 3指令
# 4       #OF    溢出                        陷阱        无        INTO指令
# 5       #BR    边界范围超出                故障        无        BOUND指令
# 6       #UD    无效操作码（未定义操作码）  故障        无        UD2指令或保留的操作码。（Pentium Pro中加入的新指令）
# 7       #NM    设备不存在（无数学协处理器）故障        无        浮点或WAIT/FWAIT指令
# 8       #DF    双重错误                    异常终止    有（0）   任何可产生异常、NMI或INTR的指令
# 9        --    协处理器段超越（保留）      故障        无        浮点指令（386以后的CPU不产生该异常）
# 10      #TS    无效的任务状态段TSS         故障        有        任务交换或访问TSS
# 11      #NP    段不存在                    故障        有        加载段寄存器或访问系统段
# 12      #SS    堆栈段错误                  故障        有        堆栈操作和SS寄存器加载
# 13      #GP    一般保护错误                故障        有        任何内存引用和其他保护检查
# 14      #PF    页面错误                    故障        有        任何内存引用
# 15       --    （Intel保留，请勿使用）                 无 
# 16      #MF    x87 FPU浮点错误（数学错误） 故障        无        x87 FPU浮点或WAIT/FWAIT指令
# 17      #AC    对齐检查                    故障        有（0）   对内存中任何数据的引用
# 18      #MC    机器检查                    异常终止    无        错误码（若有）和产生源与CPU类型有关（奔腾处理器引进）
# 19      #XF    SIMD浮点异常                故障        无        SSE和SSE2浮点指令（PIII处理器引进）
# 20-31    --    （Intel保留，请勿使用）
# 32-255   --    用户定义（非保留）中断      中断                  外部中断或者INT n指令
# ---------------------------------------------------------------------------------------------------------------------

.global isr_vector

int_service_routine:
    pushal
    push    %ds
    push    %es
    push    %fs
    push    %gs

    movl    $(SEG_KDATA<<3),%edx
    movw    %dx,            %ds
    movw    %dx,            %es
    movw    %dx,            %fs
    movw    %dx,            %gs

    movl    48(%esp),       %eax
    pushl   %esp
    call    do_irq
    addl    $4,             %esp

    pop     %gs
    pop     %fs
    pop     %es
    pop     %ds
    popal
    addl    $8,             %esp
    iret


#define MAKE_ISR(index) \
isr_##index:				\
	pushl	$0x00;			\
	pushl	$index;			\
	jmp		int_service_routine

#define MAKE_ISR_WITH_ERROR(index) \
isr_##index:				\
	pushl	$index;			\
	jmp		int_service_routine

MAKE_ISR(0x00)
MAKE_ISR(0x01)
MAKE_ISR(0x02)
MAKE_ISR(0x03)
MAKE_ISR(0x04)
MAKE_ISR(0x05)
MAKE_ISR(0x06)
MAKE_ISR(0x07)
MAKE_ISR_WITH_ERROR(0x08)
MAKE_ISR(0x09)
MAKE_ISR_WITH_ERROR(0x0a)
MAKE_ISR_WITH_ERROR(0x0b)
MAKE_ISR_WITH_ERROR(0x0c)
MAKE_ISR_WITH_ERROR(0x0d)
MAKE_ISR_WITH_ERROR(0x0e)
MAKE_ISR(0x0f)
MAKE_ISR_WITH_ERROR(0x10)
MAKE_ISR(0x11)
MAKE_ISR(0x12)
MAKE_ISR(0x13)
MAKE_ISR(0x14)
MAKE_ISR(0x15)
MAKE_ISR(0x16)
MAKE_ISR(0x17)
MAKE_ISR(0x18)
MAKE_ISR(0x19)
MAKE_ISR(0x1a)
MAKE_ISR(0x1b)
MAKE_ISR(0x1c)
MAKE_ISR(0x1d)
MAKE_ISR(0x1e)
MAKE_ISR(0x1f)
MAKE_ISR(0x20)
MAKE_ISR(0x21)
MAKE_ISR(0x22)
MAKE_ISR(0x23)
MAKE_ISR(0x24)
MAKE_ISR(0x25)
MAKE_ISR(0x26)
MAKE_ISR(0x27)
MAKE_ISR(0x28)
MAKE_ISR(0x29)
MAKE_ISR(0x2a)
MAKE_ISR(0x2b)
MAKE_ISR(0x2c)
MAKE_ISR(0x2d)
MAKE_ISR(0x2e)
MAKE_ISR(0x2f)

isr_vector:
.long	isr_0x00, isr_0x01, isr_0x02, isr_0x03
.long	isr_0x04, isr_0x05, isr_0x06, isr_0x07
.long	isr_0x08, isr_0x09, isr_0x0a, isr_0x0b
.long	isr_0x0c, isr_0x0d, isr_0x0e, isr_0x0f
.long	isr_0x10, isr_0x11, isr_0x12, isr_0x13
.long	isr_0x14, isr_0x15, isr_0x16, isr_0x17
.long	isr_0x18, isr_0x19, isr_0x1a, isr_0x1b
.long	isr_0x1c, isr_0x1d, isr_0x1e, isr_0x1f
.long	isr_0x20, isr_0x21, isr_0x22, isr_0x23
.long	isr_0x24, isr_0x25, isr_0x26, isr_0x27
.long	isr_0x28, isr_0x29, isr_0x2a, isr_0x2b
.long	isr_0x2c, isr_0x2d, isr_0x2e, isr_0x2f

