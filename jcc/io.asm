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
