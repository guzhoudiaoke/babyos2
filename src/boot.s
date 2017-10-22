#
# guzhoudiaoke@126.com
# 2017-10-21
#

.include "kernel.inc"

.code16
.global _start


_start:
	jmp main

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

	#save video info
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
    .word   0x01                        # [2] blocks to read
    .word   0x00                        # [4] transfer buffer(16 bit offset)
    .word   0x00                        # [6] transfer buffer(16 bit segment)
    .long   0x01                        # [8] starting LBA
    .long   0x00                        # [12]used for upper part of 48 bit LBAs

read_a_sect_hd:
    lea     disk_addr_packet,   %si
    movb    $0x42,              %ah
    movb    $0x80,              %dl
    int     $0x13
    ret

load_kernel:
    lea     disk_addr_packet,   %si
    movw    $TMP_KERNEL_ADDR>>4,6(%si)
    xorw    %cx,                %cx

1:
    call    read_a_sect_hd

    lea     disk_addr_packet,   %si
    movl    8(%si),             %eax
    addl    $0x01,              %eax
    movl    %eax,               (disk_addr_packet + 8)

    movl    6(%si),             %eax
    addl    $512>>4,            %eax
    movl    %eax,               (disk_addr_packet + 6)

	incw	%cx
	cmpw	$KERNEL_SECT_NUM+1,	%cx
	jne		1b

	# move first sector(the loader) to 0x0000
	cld									# si, di increment
	movw	$TMP_KERNEL_ADDR>>4,%ax
	movw	%ax,				%ds		# DS:SI src
	xorw	%si,				%si
	movw	$0x00,				%ax
	movw	%ax,				%es		# ES:DI dst
	xorw	%di,				%di
	movw	$SECT_SIZE >> 1,	%cx		# 512/2 times 
	rep		movsl						# 4 bytes per time

    ret

# 
copy_gdt_and_video_info:
	xorw	%ax,						%ax
	movw	%ax,						%ds		# DS:SI 为源地址
	leaw	gdt,						%si
	movw	$GDT_ADDR >> 4,				%ax		# 由要保存的地址来计算段基址
	movw	%ax,						%es		# ES:DI 为目的地址
	xorw	%di,						%di
	movw	$(GDT_SIZE+VIDEO_INFO_SIZE),%cx		# 移动的双字个数
	rep		movsb

	ret


begin_protected_mode:
	cli
	lgdt	gdt_ptr

enable_a20:
	inb		$0x64,			%al			# 从端口0x64读取数据
	testb	$0x02,			%al			# 测试读取数据第二个bit
	jnz		enable_a20					# 忙等待

	movb	$0xdf,			%al
	outb	%al,			$0x60		# 将0xdf写入端口0x60

	movl	%cr0,			%eax		# 读取cr0寄存器
	orl		$0x01,			%eax		# 置位最后以为即PE位
	movl	%eax,			%cr0		# 写cr0寄存器

	ljmp	$CODE_SELECTOR,	$0x00		# 跳转到代码段，即load.s处开始执行
	
	ret 
#1:
#	inb		$0x64,	%al
#	testb	$0x02,	%al
#	jnz		1b
#
#	movb	$0xd1,	%al
#	outb	%al,	$0x64
#
#2:
#	inb		$0x64,	%al
#	testb	$0x02,	%al
#	jnz		2b
#
#	movb	$0xdf,	%al
#	outb	%al,	$0x60
#
#	lgdt	gdt_ptr
#	movl	%cr0,	%eax
#	orl		$0x01,	%eax
#	movl	%eax,	%cr0
#
#	ljmp	$CODE_SELECTOR, $0x00
#
#	ret
#    
main:
	xorw	%ax,			%ax
	movw	%ax,			%ds
	movw	%ax,			%es
	movw	%ax,			%ss
	movw	$STACK_BOOT,	%sp

	call	clear_screen
	call	set_video_mode
	call	load_kernel
	call	copy_gdt_and_video_info
	call	begin_protected_mode

1:
	jmp		1b

.p2align 2
gdt:
.quad	0x0000000000000000
.quad	0x00cf9a000000ffff
.quad	0x00cf92000000ffff
.quad	0x0000000000000000
.quad	0x0000000000000000

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
gdt_ptr:
.word	video_mode - gdt - 1
.long	GDT_ADDR

.org	0x1fe,	0x90	# nop
.word	0xaa55

