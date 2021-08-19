_start:
	push	rbp
	mov	rsp -> rbp
	addi	rsp, 8
	movi	3 -> r0
	mov	rbp -> r1
	addi	r1, 0
	drefw	r0 -> r1
	mov	rbp -> r2
	addi	r2, 0
	drefr	r2 -> r1
	movi	3 -> r2
	cmp	r1, r2 -> r1
	jz	lab_000, r1
	mov	rbp -> r3
	addi	r3, 0
	drefr	r3 -> r2
	out	r2
lab_000:
	subi	rsp, 8
	pop	rbp
	ret
