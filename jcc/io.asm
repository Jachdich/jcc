println:
    push r0
    push r1
    mov rsp -> r0
    subi r0, 12
    drefr r0 -> r1
    out r1
    pop r1
    pop r2
    ret