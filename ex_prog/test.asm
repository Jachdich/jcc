_start:
    movi 6 -> r1
    mov rbp -> r0
    addi r0, 4
    drefw r1 -> r0
    addi rsp, 8
    movi 0 -> r0
    movi 0 -> r1
    mov rbp -> r0
    addi r0, 4
    drefr r0 -> r1
    out r1
    ret