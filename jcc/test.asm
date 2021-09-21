fizbuz:
	pushq	rbp
	mov	rsp -> rbp
	addi	rsp, 4
	movbpq	-8 -> r0
	movi	15 -> r1
	mod	r0, r1 -> r0
	movi	0 -> r1
	cmp	r0, r1 -> r0
	jz	L001, r0
	movi	10 -> r1
	pushb	r1
	call	println
	subi	rsp, 4
	jp L002
L001:
	movbpq	-8 -> r2
	movi	5 -> r3
	mod	r2, r3 -> r2
	movi	0 -> r3
	cmp	r2, r3 -> r2
	jz	L003, r2
	movi	1 -> r3
	pushb	r3
	call	println
	subi	rsp, 4
	jp L004
L003:
	movbpq	-8 -> r4
	movi	3 -> r5
	mod	r4, r5 -> r4
	movi	0 -> r5
	cmp	r4, r5 -> r4
	jz	L005, r4
	movi	0 -> r5
	pushb	r5
	call	println
	subi	rsp, 4
	jp L006
L005:
	movbpq	-8 -> r6
	pushq	r6
	call	println
	subi	rsp, 4
L006:
L004:
L002:
L000:
	subi	rsp, 4
	popq	rbp
	ret
_start:
	pushq	rbp
	mov	rsp -> rbp
	addi	rsp, 4
	call	func_proto
	subi	rsp, 0
	movi	0 -> r0
	movpbq	r0 -> 0
L008:
	movbpq	0 -> r1
	movi	100 -> r2
	lt	r1, r2 -> r1
	jz	L009, r1
	movbpq	0 -> r2
	pushq	r2
	call	fizbuz
	subi	rsp, 4
	movbpq	0 -> r3
	movi	1 -> r4
	add	r3, r4 -> r3
	movpbq	r3 -> 0
	jp	L008
L009:
L007:
	subi	rsp, 4
	popq	rbp
	ret
func_proto:
	pushq	rbp
	mov	rsp -> rbp
	addi	rsp, 0
	movi	69 -> r0
	pushb	r0
	call	println
	subi	rsp, 4
L010:
	subi	rsp, 0
	popq	rbp
	ret
