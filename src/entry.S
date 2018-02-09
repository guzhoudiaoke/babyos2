#
# guzhoudiaoke@126.com
# 2017-10-22
#

#include "kernel.h"

.global _start
_start = entry - KERNEL_BASE

.global entry
entry:
	# 1. clear pg_dir, pg_table0 and pg_table_vram
	xorl %eax,					%eax
	movl $entry_pg_dir,			%edi
	movl $1024,					%ecx
	cld
	rep
	stosl

	xorl %eax,					%eax
	movl $entry_pg_table0,		%edi
	movl $1024,					%ecx
	cld
	rep
	stosl

	xorl %eax,					%eax
	movl $entry_pg_table_vram,	%edi
	movl $1024,					%ecx
	cld
	rep
	stosl

	# 2. set pg_dir[0] and pg_dir[0xc0000000/4M*4] as pg_table0
	movl $(entry_pg_table0-KERNEL_BASE),	%ebx
	orl  $(PTE_P|PTE_W),					%ebx
	movl %ebx,								(entry_pg_dir-KERNEL_BASE)
	movl %ebx,								(entry_pg_dir-KERNEL_BASE) + (KERNEL_BASE >> 20)


	# 3 set pg_dir[VRAM >> 22] = pg_table_vram
	movl $(entry_pg_table_vram-KERNEL_BASE),%ebx
	orl  $(PTE_P|PTE_W),					%ebx
	movl $(entry_pg_dir-KERNEL_BASE),		%eax
	movl (VIDEO_INFO_ADDR+8),				%edx
	shrl  $20,								%edx
	addl %edx,								%eax
	movl %ebx,								(%eax)

	# 4. set pg_table0[] = {0, 4k, 8k, 12k ... 4M-4k} | (PTE_P | PTE_W)
	cld
	movl $(PTE_P|PTE_W),					%eax
	movl $1024,								%ecx
	movl $(entry_pg_table0-KERNEL_BASE),	%edi
1:
	stosl
	addl $0x1000,							%eax
	decl %ecx
	jg   1b

	# 5. set pg_table_vram[] = {VRAM, VRAM+4k, VRAM+8k, VRAM+12k ... } | (PTE_P | PTE_W)
	cld
	movl (VIDEO_INFO_ADDR + 8),				%eax
	xorl $(PTE_P|PTE_W),					%eax
	movl $1024,								%ecx
	movl $(entry_pg_table_vram-KERNEL_BASE),%edi
2:
	stosl
	addl $0x1000,							%eax
	decl %ecx
	jg   2b

	# 6. setup page directory
	movl $(entry_pg_dir-KERNEL_BASE),		%eax
	movl %eax,								%cr3

	# 7. turn on paging
	movl %cr0,								%eax
	orl	 $(CR0_PG),							%eax
	movl %eax,								%cr0

	# 8. set a new stack
	movl $(kernel_stack + KSTACK_SIZE),		%esp

	# 9. jump to main
	mov $main,								%eax
	jmp *%eax

