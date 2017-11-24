/*
 * guzhoudiaoke@126.com
 * 2017-10-26
 */


#include "kernel.h"

common_isr:
	pushal
	push	%ds
	push	%es
	push	%fs
	push	%gs

	movl	$(SEG_KDATA << 3),		%edx
	movw	%dx,					%ds
	movw	%dx,					%es

	pushl	%esp
	call	do_isr
	addl	$4,						%esp

	pop		%gs
	pop		%fs
	pop		%es
	pop		%ds
	popal
	addl	$8,						%esp			# jmp the address of isr(48) and error code(52)
	iret

isr_0:
	pushl	$0
	pushl	$0
	jmp		common_isr

isr_1:
	pushl	$0
	pushl	$1
	jmp		common_isr
	
isr_2:
	pushl	$0
	pushl	$2
	jmp		common_isr

isr_3:
	pushl	$0
	pushl	$3
	jmp		common_isr

isr_4:
	pushl	$0
	pushl	$4
	jmp		common_isr

isr_5:
	pushl	$0
	pushl	$5
	jmp		common_isr

isr_6:
	pushl	$0
	pushl	$6
	jmp		common_isr

isr_7:
	pushl	$0
	pushl	$7
	jmp		common_isr

isr_8:
	pushl	$0
	pushl	$8
	jmp		common_isr

isr_9:
	pushl	$0
	pushl	$9
	jmp		common_isr

isr_10:
	pushl	$0
	pushl	$10
	jmp		common_isr

isr_11:
	pushl	$0
	pushl	$11
	jmp		common_isr

isr_12:
	pushl	$0
	pushl	$12
	jmp		common_isr

isr_13:
	pushl	$0
	pushl	$13
	jmp		common_isr

isr_14:
	pushl	$0
	pushl	$14
	jmp		common_isr

isr_15:
	pushl	$0
	pushl	$15
	jmp		common_isr

isr_16:
	pushl	$0
	pushl	$16
	jmp		common_isr

isr_17:
	pushl	$0
	pushl	$17
	jmp		common_isr

isr_18:
	pushl	$0
	pushl	$18
	jmp		common_isr

isr_19:
	pushl	$0
	pushl	$19
	jmp		common_isr

isr_20:
	pushl	$0
	pushl	$20
	jmp		common_isr

isr_21:
	pushl	$0
	pushl	$21
	jmp		common_isr

isr_22:
	pushl	$0
	pushl	$22
	jmp		common_isr

isr_23:
	pushl	$0
	pushl	$23
	jmp		common_isr

isr_24:
	pushl	$0
	pushl	$24
	jmp		common_isr
	
isr_25:
	pushl	$0
	pushl	$25
	jmp		common_isr

isr_26:
	pushl	$0
	pushl	$26
	jmp		common_isr

isr_27:
	pushl	$0
	pushl	$27
	jmp		common_isr

isr_28:
	pushl	$0
	pushl	$28
	jmp		common_isr

isr_29:
	pushl	$0
	pushl	$29
	jmp		common_isr

isr_30:
	pushl	$0
	pushl	$30
	jmp		common_isr

isr_31:
	pushl	$0
	pushl	$31
	jmp		common_isr

isr_32:
	pushl	$0
	pushl	$32
	jmp		common_isr

isr_33:
	pushl	$0
	pushl	$33
	jmp		common_isr

isr_34:
	pushl	$0
	pushl	$34
	jmp		common_isr

isr_35:
	pushl	$0
	pushl	$35
	jmp		common_isr

isr_36:
	pushl	$0
	pushl	$36
	jmp		common_isr

isr_37:
	pushl	$0
	pushl	$37
	jmp		common_isr

isr_38:
	pushl	$0
	pushl	$38
	jmp		common_isr

isr_39:
	pushl	$0
	pushl	$39
	jmp		common_isr

isr_40:
	pushl	$0
	pushl	$40
	jmp		common_isr

isr_41:
	pushl	$0
	pushl	$41
	jmp		common_isr

isr_42:
	pushl	$0
	pushl	$42
	jmp		common_isr

isr_43:
	pushl	$0
	pushl	$43
	jmp		common_isr

isr_44:
	pushl	$0
	pushl	$44
	jmp		common_isr

isr_45:
	pushl	$0
	pushl	$45
	jmp		common_isr

isr_46:
	pushl	$0
	pushl	$46
	jmp		common_isr

isr_47:
	pushl	$0
	pushl	$47
	jmp		common_isr

.data
.global isr_vec
isr_vec:
	.long isr_0
	.long isr_1
	.long isr_2
	.long isr_3
	.long isr_4
	.long isr_5
	.long isr_6
	.long isr_7
	.long isr_8
	.long isr_9
	.long isr_10
	.long isr_11
	.long isr_12
	.long isr_13
	.long isr_14
	.long isr_15
	.long isr_16
	.long isr_17
	.long isr_18
	.long isr_19
	.long isr_20
	.long isr_21
	.long isr_22
	.long isr_23
	.long isr_24
	.long isr_25
	.long isr_26
	.long isr_27
	.long isr_28
	.long isr_29
	.long isr_30
	.long isr_31
	.long isr_32
	.long isr_33
	.long isr_34
	.long isr_35
	.long isr_36
	.long isr_37
	.long isr_38
	.long isr_39
	.long isr_40
	.long isr_41
	.long isr_42
	.long isr_43
	.long isr_44
	.long isr_45
	.long isr_46
	.long isr_47

