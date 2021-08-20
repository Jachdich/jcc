some_func:
	push	rbp
	mov	rsp -> rbp
	addi	rsp, 0
	movar	glob -> r0
	out	r0
	subi	rsp, 0
	pop	rbp
	ret
_start:
	push	rbp
	mov	rsp -> rbp
	addi	rsp, 8
	movar	glob -> r0
	out	r0
	movi	69 -> r0
	movra	r0 -> glob
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
	movi	32 -> r2
	mov	rbp -> r3
	addi	r3, 4
	drefw	r2 -> r3
	mov	rbp -> r4
	addi	r4, 0
	drefr	r4 -> r3
	out	r3
	mov	rbp -> r4
	addi	r4, 4
	drefr	r4 -> r3
	out	r3
lab_000:
	call	some_func
	subi	rsp, 8
	pop	rbp
	ret
glob: dd 32
