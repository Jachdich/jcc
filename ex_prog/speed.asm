_start:
        movl 2147483647 -> r0
        mov r0 -> r1
        divi r0, 2
        addi r0, 1
pos:
        mod r1, r0 -> r3
        jcr z, posend
        subi r0, 1
        jr pos
posend:
        subi r0, 1
        jcr z, prime
        movi 0 -> r0
        out r0
        jr end

prime:  movi 1 -> r0
        out r0
end:    halt
