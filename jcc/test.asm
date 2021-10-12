get_val:
	pushq	rbp
	mov	rsp -> rbp
	addi	rsp, 4
	movbpq	-8 -> r0
	movi	2 -> r1
	mod	r0, r1 -> r0
	movi	0 -> r1
	cmp	r0, r1 -> r0
	jz	L001, r0
	movbpq	-8 -> r1
	pushq	r1
	jp	L000
	jp L002
L001:
	movi	99 -> r1
	pushq	r1
	jp	L000
L002:
L000:
	popq	r1
	subi	rsp, 4
	popq	rbp
	pushq	r1
	ret
main:
	pushq	rbp
	mov	rsp -> rbp
	addi	rsp, 8
	movi	0 -> r0
	movpbq	r0 -> 0
L004:
	movbpq	0 -> r1
	movi	10 -> r2
	lt	r1, r2 -> r1
	jz	L005, r1
	movbpq	0 -> r2
	pushq	r2
	call	println
	subi	rsp, 4
	movbpq	0 -> r3
	movi	1 -> r4
	add	r3, r4 -> r3
	movpbq	r3 -> 0
	jp	L004
L005:
	movi	0 -> r4
	movpbq	r4 -> 4
L006:
	movbpq	4 -> r5
	movi	10 -> r6
	lt	r5, r6 -> r5
	jz	L007, r5
	movbpq	4 -> r6
	pushq	r6
	call	get_val
	popq	r7
	subi	rsp, 4
	pushq	r7
	call	println
	subi	rsp, 4
	movbpq	4 -> r8
	movi	1 -> r9
	add	r8, r9 -> r8
	movpbq	r8 -> 4
	jp	L006
L007:
	movi	0 -> r9
	pushq	r9
	jp	L003
L003:
	popq	r9
	subi	rsp, 8
	popq	rbp
	pushq	r9
	ret
