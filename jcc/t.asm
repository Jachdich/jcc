_start:
	pushq	rbp
	mov	rsp -> rbp
	addi	rsp, 4
	movi	0 -> r0
	movpbq	r0 -> 0
L001:
	movbpq	0 -> r1
	movi	10 -> r2
	lt	r1, r2 -> r1
	jz	L002, r1
	movbpq	0 -> r2
	pushq	r2
	call	println
	subi	rsp, 4
	movbpq	0 -> r3
	movi	1 -> r4
	add	r3, r4 -> r3
	movpbq	r3 -> 0
	jp	L001
L002:
L000:
	subi	rsp, 4
	popq	rbp
	ret
