add:
	push	rbp
	mov	rsp -> rbp
	addi	rsp, 2
	movbpb	-5 -> r0
	movbpb	-6 -> r1
	add	r0, r1 -> r0
	push	r0
	jp	L000
L000:
	pop	r1
	subi	rsp, 2
	pop	rbp
	push	r1
	ret
_start:
	push	rbp
	mov	rsp -> rbp
	addi	rsp, 0
	movi	-10 -> r0
	movi	10 -> r1
	push	r1
	push	r0
	call	add
	pop	r2
	subi	rsp, 8
	push	r2
	call	println
	subi	rsp, 4
L001:
	subi	rsp, 0
	pop	rbp
	ret
