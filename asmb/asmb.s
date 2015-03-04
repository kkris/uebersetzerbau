	.file	"asmb.c"
	.text
	.globl	asmb
	.type	asmb, @function
asmb:
.LFB0:
	.cfi_startproc
	testq	%rcx, %rcx
	je	.L6
	movl	$0, %eax
	movl	$0, %r9d
.L5:
	movq	(%rdi,%rax,8), %r10
	movq	%r10, %r8
	subq	(%rsi,%rax,8), %r8
	subq	%r9, %r8
	testq	%r9, %r9
	je	.L3
	cmpq	%r8, %r10
	setbe	%r9b
	movzbl	%r9b, %r9d
	jmp	.L4
.L3:
	cmpq	%r8, %r10
	setb	%r9b
	movzbl	%r9b, %r9d
.L4:
	movq	%r8, (%rdx,%rax,8)
	addq	$1, %rax
	cmpq	%rcx, %rax
	jne	.L5
	jmp	.L2
.L6:
	movl	$0, %r9d
.L2:
	negq	%r9
	movq	%r9, (%rdx,%rcx,8)
	ret
	.cfi_endproc
.LFE0:
	.size	asmb, .-asmb
	.ident	"GCC: (GNU) 4.9.2 20150204 (prerelease)"
	.section	.note.GNU-stack,"",@progbits
