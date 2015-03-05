	.globl	asma
	.type	asma, @function
asma:
.LFB0:
	.cfi_startproc
	movq	(%rdi), %rax
	subq	(%rsi), %rax
    movq    8(%rdi), %rcx
    sbbq    8(%rsi), %rcx
	movq	%rax, (%rdx)
    movq    %rcx, 8(%rdx)
	ret
	.cfi_endproc
.LFE0:
	.size	asma, .-asma
