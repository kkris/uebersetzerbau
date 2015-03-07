	.file	"template.c" # XXX
	.text
	.globl	asmb
	.type	asmb, @function
asmb:
.LFB0:
	.cfi_startproc
    mov $0, %r8
    clc # set cf flag to zero?

    movq $0, %r10

.loopbody:
    bt $0, %r10

    movq    (%rdi, %r8, 8), %rax
    sbbq    (%rsi, %r8, 8), %rax
    movq    %rax, (%rdx, %r8, 8)

    sbbq    %r10, %r10

    inc %r8
    cmpq    %rcx, %r8
    jne .loopbody
.loopend:
    movq  %r10, (%rdx, %rcx, 8)
	ret
	.cfi_endproc
.LFE0:
	.size	asmb, .-asmb
	.ident	"GCC: (GNU) 4.9.2 20150204 (prerelease)"
	.section	.note.GNU-stack,"",@progbits
