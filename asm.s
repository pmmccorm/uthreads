.globl unlock

unlock:
	pushq %rbp
	movq %rsp, %rbp

	movb $0, (%rdi)

	popq %rbp
	ret

.globl thread_switch

thread_switch:
    push %rbx
    push %rbp
    push %r12
    push %r13
    push %r14
    push %r15

    movq %rsp, (%rdi)	# save old stackp
	movb $0, (%rdx)		# release lock
    movq (%rsi), %rsp	# restore saved stackp

    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %rbp
    pop %rbx

    ret

.globl thread_switch_d

thread_switch_d:
    movq (%rdi), %rsp

    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %rbp
    pop %rbx

    ret

.globl thread_start

thread_start:
    push %rbx
    push %rbp
    push %r12
    push %r13
    push %r14
    push %r15


    movq %rsp, (%rdi)
	movb $0, (%rdx)		# release lock
    movq (%rsi), %rsp

    call uthread_wrap

.globl thread_start_d

thread_start_d:
    movq (%rdi), %rsp

    call uthread_wrap
