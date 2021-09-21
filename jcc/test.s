	.file	"test.c"
	.text
	.p2align 4
	.globl	_start
	.type	_start, @function
_start:
.LFB0:
	.cfi_startproc
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	xorl	%ebx, %ebx
	.p2align 4,,10
	.p2align 3
.L2:
	movl	%ebx, %edi
	addl	$1, %ebx
	call	println
	cmpl	$10, %ebx
	jne	.L2
	popq	%rbx
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE0:
	.size	_start, .-_start
	.ident	"GCC: (GNU) 11.1.0"
	.section	.note.GNU-stack,"",@progbits
