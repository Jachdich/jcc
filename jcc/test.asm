add:
	push	rbp
	mov	rsp -> rbp
	addi	rsp, 8
	movbp	-8 -> r0
	movbp	-12 -> r1
	sub	r0, r1 -> r0
	push	r0
	jp	L000
L000:
	pop	r1
	subi	rsp, 8
	pop	rbp
	push	r1
	ret
_start:
	push	rbp
	mov	rsp -> rbp
	addi	rsp, 0
	movi	10 -> r0
	movi	2 -> r1
	push	r1
	push	r0
	call	add
	pop	r2
	subi	rsp, 8
	movi	1 -> r3
	add	r2, r3 -> r2
	push	r2
	call	println
	subi	rsp, 4
L001:
	subi	rsp, 0
	pop	rbp
	ret
