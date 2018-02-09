#
# guzhoudiaoke@126.com
# 2017-10-21
#

#include "kernel.h"

.code16
.global _start


_start:
jmp main

# function to clear the screen
clear_screen:
	movb	$0x06,	%ah
	movb	$0x00,	%al		# roll up all rows, clear the screen
	movb	$0x00,	%ch		# row of left top corner
	movb	$0x00,	%cl		# col of left top corner
	movb	$0x18,	%dh		# row of right bottom corner
	movb	$0x4f,	%dl		# col of right bottom corner
	movb	$0x07,	%bh		# property of roll up rows
	int		$0x10
ret

# function to set video mode
set_video_mode:
	xorw	%ax,		%ax
	movw	%ax,		%ds
	movw	%ax,		%es
	movw	$0x800,		%di	# buffer

	# check vbe
	movw	$0x4f00,	%ax
	int     $0x10

	cmp		$0x004f,	%ax
	jne		set_vga_0x13

	movw	0x04(%di),	%ax
	cmp		$0x0200,	%ax	# vbe version < 2.0
	jb		set_vga_0x13

	# check vbe mode 0x118
	movw	$0x118,		%cx
	movw	$0x4f01,	%ax
	int		$0x10

	cmpb	$0x00,		%ah	# call failed
	jne		set_vga_0x13

	cmpb	$0x4f,		%al # not support this mode
	jne		set_vga_0x13

	movw	(%di),		%ax
	andw	$0x0080,	%ax	# not support Linear Frame Buffer memory model
	jz		set_vga_0x13

	# save video info
	movw    $0x118,		video_mode
	movw	0x12(%di),	%ax
	movw	%ax,		screen_x
	movw	0x14(%di),	%ax
	movw	%ax,		screen_y
	movb	0x19(%di),	%al
	movb	%al,		bits_per_pixel
	movb	0x1b(%di),	%al
	movb	%al,		memory_model
	movl	0x28(%di),	%eax
	movl	%eax,		video_ram

	#set vbe mode
	movw	$0x118,		%bx
	addw	$0x4000,	%bx
	movw	$0x4f02,	%ax
	int		$0x10
	ret

set_vga_0x13:
	movb	$0,			%ah
	movb	$0x13,		%al
	int		$0x10
	ret


# read kernel from hd
# and put loader to 0x0000
disk_addr_packet:
    .byte   0x10                        # [0] size of packet 16 bytes
    .byte   0x00                        # [1] reserved always 0
    .word   0x00                        # [2] blocks to read
    .word   0x00                        # [4] transfer buffer(16 bit offset)
    .word   0x00                        # [6] transfer buffer(16 bit segment)
    .long   0x00                        # [8] starting LBA
    .long   0x00                        # [12]used for upper part of 48 bit LBAs

# function to read a sect from hd
read_a_sect_hd:

# function to load the kernel
load_loader:
    lea     disk_addr_packet,   %si
    movw    $2,                 2(%si)
    movw    $1,                 8(%si)
    movb    $0x42,              %ah
    movb    $0x80,              %dl
    int     $0x13
    ret


# function to copy gdt and video info to a safe position
copy_gdt_and_video_info:
	xorw	%ax,						%ax
	movw	%ax,						%ds		# DS:SI src
	leaw	video_mode,					%si
	movw	$BOOT_INFO_SEG,				%ax
	movw	%ax,						%es		# ES:DI dst
	xorw	%di,						%di
	movw	$(VIDEO_INFO_SIZE),         %cx		# num of bytes to move
	rep		movsb

	ret

# finnction to get memory info by 0xe820
get_memory_info:
	movw	$(BOOT_INFO_SEG),		%ax
	movw	%ax,					%es
	movw	%ax,					%ds

	xorl	%ebx,					%ebx
	movl	$(MEMORY_INFO_OFFSET),	%edi
	addl	$4,						%edi
	movl	$0,						(MEMORY_INFO_OFFSET)

1:
	movl	$0xe820,				%eax
	movl	$20,					%ecx
	movl	$0x534d4150,			%edx
	int		$0x15
	jc		failed

	addl	$20,					%edi
	incl	(MEMORY_INFO_OFFSET)
	cmpl	$0,						%ebx
	jne		1b
	jmp		success

failed:
	movl	$0,						(MEMORY_INFO_OFFSET)

success:
	xorw	%ax,					%ax
	movw	%ax,					%es
	movw	%ax,					%ds

	ret


# function to begin protected mode
begin_protected_mode:
	cli
1:
	inb		$0x64,			%al
	testb	$0x02,			%al
	jnz		1b

	movb	$0xd1,			%al
	outb	%al,			$0x64

2:
	inb		$0x64,			%al
	testb	$0x02,			%al
	jnz		2b

	movb	$0xdf,			%al
	outb	%al,			$0x60

	lgdt	gdt_ptr
	movl	%cr0,			%eax
	orl		$CR0_PE,		%eax
	movl	%eax,			%cr0

	ljmp	$SEG_KCODE<<3,	$0
	ret
    
# the main function
main:
	xorw	%ax,			%ax
	movw	%ax,			%ds
	movw	%ax,			%es
	movw	%ax,			%ss
	movw	$STACK_BOOT,	%sp

	call	clear_screen
	call	set_video_mode
	call	get_memory_info
	call	load_loader
	call	copy_gdt_and_video_info
	call	begin_protected_mode

1:
	jmp		1b


.p2align 2
gdt:
.quad	0x0000000000000000
.quad	0x00cf9a000000ffff
.quad	0x00cf92000000ffff
gdt_ptr:
.word	. - gdt - 1
.long   gdt

video_mode:
.short  0
screen_x:
.short	0 
screen_y:
.short	0	
bits_per_pixel:
.byte   0
memory_model:
.byte   0
video_ram:
.long	0

.org	0x1fe,	0x90	# nop
.word	0xaa55

