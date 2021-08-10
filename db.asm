a: dd 59
_start:
    movar a -> r2
    out r2
	movl	3 -> r0
	movra	r0 -> a
	movar a -> r1
	out r1
	ret
