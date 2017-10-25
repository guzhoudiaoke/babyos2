#
# guzhoudiaoke@126.com
# 2017-10-22
#

#include "kernel.h"

.global _start
_start = entry - 0xc0000000

.global entry
entry:
	# 1. clear pg_dir and pg0
	xorl %eax,				%eax
	movl $entry_pg_dir,		%edi
	movl $1024,				%ecx
	cld
	rep
	stosl

	xorl %eax,				%eax
	movl $entry_pg_table0,	%edi
	movl $1024,				%ecx
	cld
	rep
	stosl

	# 2. set pg_dir[0] and pg_dir[0xc0000000/4M*4] as pte0
	movl $(entry_pg_table0-KERNEL_BASE),	%ebx
	orl  $(PTE_P|PTE_W),					%ebx
	movl %ebx,								(entry_pg_dir-KERNEL_BASE)
	addl (KERNEL_BASE >> 20 << 2),			%eax
	movl %ebx,								(entry_pg_dir-KERNEL_BASE) + (KERNEL_BASE >> 20)

	# 3. set pte0[] = {0, 4k, 8k, 12k ... 4M-4k} | (PTE_P | PTE_W)
	cld
	movl $(PTE_P|PTE_W),					%eax
	movl $1024,								%ecx
	movl $(entry_pg_table0-KERNEL_BASE),	%edi
1:
	stosl
	addl $0x1000,							%eax
	decl %ecx
	jg   1b

	# 4. setup page directory
	movl $(entry_pg_dir-KERNEL_BASE),		%eax
	movl %eax,								%cr3

	# 5. turn on paging
	movl %cr0,								%eax
	orl	 $(CR0_PG),							%eax
	movl %eax,								%cr0

	# 6. set a new stack
	movl $kernel_stack,						%esp

	# 7. jump to main
	mov $main,								%eax
	jmp *%eax

