_start:
    movi 0 -> r0
    jz b, r0
    movi 1 -> r0
    out r0
    jp end

b:  
    movi 2 -> r0
    out r0

end:
    ret
