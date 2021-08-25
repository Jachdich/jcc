var: dd 0
_start:
    movi 500 -> r0
    movrad r0 -> var
    movl var -> r2
    addi r2, 1
    drefrb r2 -> r3
    addi r3, 1
    drefwb r3 -> r2
    movard var -> r1
    out r1
    ret