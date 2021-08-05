        movl 2147483647 -> r0
        mov r0 -> r1
        movi 2 -> r2
        divi r0, 2 -> r0
        addi r0, 1 -> r0
pos:
        mod r1, r2 -> r3
        jcr nz, pos
        
        sub r2, r1 -> r3
        jcr z, false
        movi 1 -> r0
        out r0
        jr end
false:  movi 0 -> r0
        out r0
end:    halt
