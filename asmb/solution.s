	.text
	.globl	asmb
	.type	asmb, @function
asmb:
.LFB0:
	.cfi_startproc
    mov $0, %r8
    mov %rcx, %r9
    clc # set cf flag to zero?

    testq %rcx, %rcx # n = 0
    je .loopend

.loopbody:
    movq    (%rdi, %r8, 8), %rax
    sbbq    (%rsi, %r8, 8), %rax
    movq    %rax, (%rdx, %r8, 8)

    inc %r8
    dec %r9
    jnz .loopbody
.loopend:
    sbbq %r10, %r10
    movq %r10, (%rdx, %rcx, 8)
	ret
	.cfi_endproc
.LFE0:
	.size	asmb, .-asmb
