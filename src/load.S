#
# guzhoudiaoke@126.com
# 2017-10-21
#

#include "kernel.h"

.section .text
.global _start

.org 0

_start:
	jmp		main


main:
	movl	$(SEG_KDATA<<3),		%eax
	movw	%ax,					%ds
	movw	%ax,					%es
	movw	%ax,					%fs
	movw	%ax,					%gs
	movw	%ax,					%ss
	movl	$STACK_PM_BOTTOM,		%esp

	call	loadmain

1:
	jmp		1b
