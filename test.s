	.file	"test.c"
	.text
	.type	uthread_spinlock, @function
uthread_spinlock:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -8(%rbp)
	nop
.L2:
	movq	-8(%rbp), %rdx
	movl	$1, %eax
	xchgb	(%rdx), %al
	testb	%al, %al
	jne	.L2
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	uthread_spinlock, .-uthread_spinlock
	.section	.tdata,"awT",@progbits
	.align 4
	.type	x, @object
	.size	x, 4
x:
	.long	90
	.section	.tbss,"awT",@nobits
	.align 4
	.type	s_test, @object
	.size	s_test, 8
s_test:
	.zero	8
	.text
	.globl	f1
	.type	f1, @function
f1:
.LFB2:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movq	%rdi, -16(%rbp)
	movl	-16(%rbp), %edx
	movl	-12(%rbp), %eax
	addl	%edx, %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE2:
	.size	f1, .-f1
	.globl	g1
	.type	g1, @function
g1:
.LFB3:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	movl	%edi, %ecx
	movl	%esi, %eax
	movl	%edx, -12(%rbp)
	movb	%cl, -4(%rbp)
	movb	%al, -8(%rbp)
	movl	-12(%rbp), %eax
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE3:
	.size	g1, .-g1
	.section	.rodata
.LC0:
	.string	"%lu\n"
	.text
	.globl	f2
	.type	f2, @function
f2:
.LFB4:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$224, %rsp
	movq	%rdi, -176(%rbp)
	movq	%rsi, -168(%rbp)
	movq	%rdx, -160(%rbp)
	movq	%rcx, -152(%rbp)
	movq	%r8, -144(%rbp)
	movq	%r9, -136(%rbp)
	testb	%al, %al
	je	.L8
	movaps	%xmm1, -112(%rbp)
	movaps	%xmm2, -96(%rbp)
	movaps	%xmm3, -80(%rbp)
	movaps	%xmm4, -64(%rbp)
	movaps	%xmm5, -48(%rbp)
	movaps	%xmm6, -32(%rbp)
	movaps	%xmm7, -16(%rbp)
.L8:
	movss	%xmm0, -212(%rbp)
	movl	$0, -200(%rbp)
	movl	$64, -196(%rbp)
	leaq	16(%rbp), %rax
	movq	%rax, -192(%rbp)
	leaq	-176(%rbp), %rax
	movq	%rax, -184(%rbp)
	jmp	.L9
.L12:
	movq	-208(%rbp), %rax
	movq	%rax, %rsi
	movl	$.LC0, %edi
	movl	$0, %eax
	call	printf
	movl	-200(%rbp), %eax
	cmpl	$48, %eax
	jnb	.L10
	movq	-184(%rbp), %rax
	movl	-200(%rbp), %edx
	movl	%edx, %edx
	addq	%rdx, %rax
	movl	-200(%rbp), %edx
	addl	$8, %edx
	movl	%edx, -200(%rbp)
	jmp	.L11
.L10:
	movq	-192(%rbp), %rax
	leaq	8(%rax), %rdx
	movq	%rdx, -192(%rbp)
.L11:
	movq	(%rax), %rax
	movq	%rax, -208(%rbp)
.L9:
	cmpq	$99, -208(%rbp)
	jne	.L12
	movl	$0, %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE4:
	.size	f2, .-f2
	.section	.rodata
.LC1:
	.string	"%p\n"
.LC3:
	.string	"%d\n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB5:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$48, %rsp
	movl	%edi, -36(%rbp)
	movq	%rsi, -48(%rbp)
	movb	$0, -22(%rbp)
	movq	__environ(%rip), %rax
	movq	%rax, %rsi
	movl	$.LC1, %edi
	movl	$0, %eax
	call	printf
	movl	$1, %esi
	movl	$.LC0, %edi
	movl	$0, %eax
	call	printf
	movl	$99, %r9d
	movl	$6, %r8d
	movl	$5, %ecx
	movl	$4, %edx
	movl	$3, %esi
	movl	$2, %edi
	movss	.LC2(%rip), %xmm0
	movl	$1, %eax
	call	f2
	movl	%eax, -20(%rbp)
	movl	-36(%rbp), %eax
	movl	%eax, %edx
	movl	$103, %esi
	movl	$118, %edi
	call	g1
	movl	%eax, -20(%rbp)
	movl	$1, -16(%rbp)
	movl	$2, -12(%rbp)
	movq	-16(%rbp), %rax
	movq	%rax, %rdi
	call	f1
	movl	%eax, -20(%rbp)
	leaq	-22(%rbp), %rax
	movq	%rax, %rdi
	call	uthread_spinlock
	leaq	-22(%rbp), %rax
	movq	%rax, %rdi
	call	unlock
	movzbl	-22(%rbp), %eax
	movb	%al, -21(%rbp)
	movzbl	-21(%rbp), %eax
	movl	%eax, %esi
	movl	$.LC3, %edi
	movl	$0, %eax
	call	printf
	movl	$0, %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE5:
	.size	main, .-main
	.section	.rodata
	.align 4
.LC2:
	.long	1065353216
	.ident	"GCC: (Ubuntu 4.9.3-8ubuntu2~14.04) 4.9.3"
	.section	.note.GNU-stack,"",@progbits
