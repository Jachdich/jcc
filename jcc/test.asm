a: dd 0
_start:
	movi	18 -> r0
	movra	r0 -> a
	movar	a -> r0
	movi	14 -> r1
	lt	r0, r1 -> r0
	jz	lab_000, r0
	movi	1 -> r1
	out	r1
	jp lab_001
lab_000:
	movar	a -> r1
	movi	18 -> r2
	lt	r1, r2 -> r1
	jz	lab_002, r1
	movi	2 -> r2
	out	r2
	jp lab_003
lab_002:
	movi	3 -> r2
	out	r2
lab_003:
lab_001:
	ret
