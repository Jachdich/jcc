i: dd 0
_start:
	movi	0 -> r0
	movra	r0 -> i
lab_000:
	movar	i -> r0
	movi	10 -> r1
	lt	r0, r1 -> r0
	jz	lab_001, r0
	movar	i -> r1
	movi	2 -> r2
	mod	r1, r2 -> r1
	movi	0 -> r2
	cmp	r1, r2 -> r1
	jz	lab_002, r1
	movar	i -> r2
	out	r2
lab_002:
	movar	i -> r2
	movi	1 -> r3
	add	r2, r3 -> r2
	movra	r2 -> i
	jp	lab_000
lab_001:
	ret
