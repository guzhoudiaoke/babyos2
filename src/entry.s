#
# guzhoudiaoke@126.com
# 2017-10-22
#

#include "mm.h"

.global _start
_start = entry - 0xc0000000

.global entry
entry:
	# 1. clear pg_dir and pg0
	#movl $1024,			 %ecx
	#xorl %eax,			 %eax
	#movl $entry_pg_dir, %edi
	#cld
	#rep
	#stosl

	#movl $1024,			 %ecx
	#xorl %eax,			 %eax
	#movl $entry_pte0,	 %edi
	#cld
	#rep
	#stosl

	# 2. set pg_dir[0] and pg_dir[0xc0000000/4M*4] as pg0
	movl $((entry_pte0 - 0xc0000000)),					%ebx
	orl  $(3),							%ebx
	movl %ebx,							(entry_pg_dir - 0xc0000000)
	addl (0xc0000000 >> 20 << 2),		%eax
	movl %ebx,							((entry_pg_dir - 0xc0000000) + (0xc0000000 >> 20))

#	# 3. set pg0: (0, 4k, 8k, 12k ... 4M-4k) | (PTE_P | PTE_W)
	cld
	movl $(3),	   %eax
	movl $1024,				   %ecx
	movl $(entry_pte0 - 0xc0000000), %edi
1:
	stosl
	addl $0x1000,		       %eax
	decl %ecx
	jg   1b


	movl $((entry_pg_dir - 0xc0000000)),			%eax
	movl %eax,					%cr3

	movl %cr0,					%eax
	orl	 $(0x80000000),			%eax
	movl %eax,					%cr0

	#call main
	mov $main, %eax
	jmp *%eax

