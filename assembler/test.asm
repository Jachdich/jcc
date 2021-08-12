_start:
    movi 3 -> r0
    jcr nc, 0
    jcr z, 1
    halt
