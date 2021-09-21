_start:
    alloc r0, 4
    drefr r0 -> r1
    out r1
    movi 69 -> r1
    drefw r1 -> r0
    drefr r0 -> r2
    out r2
    free r0
    ret