println:
    pushq r0
    pushq r1
    mov rsp -> r0
    subi r0, 12
    drefrq r0 -> r1
    out r1
    popq r1
    popq r2
    ret

_start:
    call main
    halt

malloc:
    addi rsp, 4
    pushq r0
    pushq r1
    pushq r2
    mov rsp -> r0
    subi r0, 20
    drefrq r0 -> r1
    alloc r1, r2
    addi r0, 4
    drefwq r2 -> r0
    popq r2
    popq r1
    popq r0
