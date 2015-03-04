	.file	"template.c" # XXX
	.text
	.globl	asmb
	.type	asmb, @function
asmb:
.LFB0:
	.cfi_startproc
    mov $0, %r8
    clc # set cf flag to zero?

    pushfq

.loopbody:
    popfq

    movq    (%rdi, %r8, 8), %rax
    sbbq    (%rsi, %r8, 8), %rax
    movq    %rax, (%rdx, %r8, 8)

    setbe %r9b
    movzbl %r9b, %r9d
    pushfq

    inc %r8
    cmpq    %rcx, %r8
    jne .loopbody
.loopend:
    popfq
    negq  %r9
    movq  %r9, (%rdx, %rcx, 8)
	ret
	.cfi_endproc
.LFE0:
	.size	asmb, .-asmb
	.ident	"GCC: (GNU) 4.9.2 20150204 (prerelease)"
	.section	.note.GNU-stack,"",@progbits
