_start:
	movl	4 -> r0
	movl	2 -> r1
	div	r0, r1 -> r0
	movl	3 -> r1
	add	r1, r0 -> r1
	out r1
    halt