	.file	"template.c"
	.text
	.globl	asma
	.type	asma, @function
asma:
.LFB0:
	.cfi_startproc
	movq	(%rdi), %rax
	subq	(%rsi), %rax
    movq    8(%rdi), %rcx
    sbbq    8(%rsi), %rcx
	movq	%rax, (%rdx)  # needed?
    movq    %rcx, 8(%rdx) # needed?
	ret
	.cfi_endproc
.LFE0:
	.size	asma, .-asma
	.ident	"GCC: (GNU) 4.9.2 20150204 (prerelease)"
	.section	.note.GNU-stack,"",@progbits
