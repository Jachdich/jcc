default rel
extern printf
section .rodata
	format db "%d", 10, 0
section .text
global main
printint:
	push rbp
	mov rbp, rsp
	sub rsp, 16
	mov [rbp - 4], edi
	mov eax, [rbp - 4]
	mov esi, eax
	lea rdi, [rel format]
	xor eax, eax
	call printf
	nop
	leave
	ret
main:
	push rbp
	mov rbp, rsp
	mov	r8, 4
	mov	r9, 2
	mov rax, r8
	cqo
	idiv	r9
	mov r8, rax
	mov	r9, 6
	add	r9, r8
	mov	r8, 5
	imul	r8, r9
	mov	rdi, r8
	call	printint
	xor eax, eax
	pop rbp
	ret
