	.text
	.globl	asmb
	.type	asmb, @function
asmb:
.LFB0:
	.cfi_startproc
    movq $0, %r8

    testq %rcx, %rcx # test if n = 0, carry flag is cleared
    je .loopend
.loopbody:
    movq    (%rdi, %r8, 8), %rax
    sbbq    (%rsi, %r8, 8), %rax
    movq    %rax, (%rdx, %r8, 8)

    inc %r8
    loop .loopbody
.loopend:
    sbbq %r10, %r10
    movq %r10, (%rdx, %r8, 8)
	ret
	.cfi_endproc
.LFE0:
	.size	asmb, .-asmb
