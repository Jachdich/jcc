_start:
	push	rbp
	mov	rsp -> rbp
	addi	rsp, 0
	movi	10 -> r0
	push	r0
	call	println
	subi	rsp, 4
	subi	rsp, 0
	pop	rbp
	ret
