# This file is part of the MicroPython port to LEON platforms
# Copyright (c) 2016 George Robotics Limited
#
# Provided to the European Space Agency as part of the project "Porting of
# MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.

    .file "gchelper.s"
    .section ".text"
    .align 4
    .global gc_helper_get_regs
    .type gc_helper_get_regs, #function

# This function gets all of the SPARC registers and stores them into the
# buffer provided as the first argument.
#
# This is a leaf function:
#   - argument is in %o0
#   - return value goes in %o0
#   - can only clobber %oX registers
gc_helper_get_regs:
    # save %wim
    mov %wim, %o1

    # save %g1 and make it point to the buffer
    mov %g1, %o2
    mov %o0, %g1

    # count number of register windows (currently unused code)
    #mov 0xfff, %wim -- should be 0xffffffff
    #nop; nop; nop
    #mov %wim, %o0 -- %o0 has a bit set for each register window

    # disable window traps
    mov %g0, %wim
    nop; nop; nop

    # save registers to given buffer
    # this is hard coded for 8 register windows
    std %l0, [%g1 + 0]
    std %l2, [%g1 + 8]
    std %l4, [%g1 + 16]
    std %l6, [%g1 + 24]
    std %i0, [%g1 + 32]
    std %i2, [%g1 + 40]
    std %i4, [%g1 + 48]
    std %i6, [%g1 + 56]
    save
    add %g1, 64, %g1
    std %l0, [%g1 + 0]
    std %l2, [%g1 + 8]
    std %l4, [%g1 + 16]
    std %l6, [%g1 + 24]
    std %i0, [%g1 + 32]
    std %i2, [%g1 + 40]
    std %i4, [%g1 + 48]
    std %i6, [%g1 + 56]
    save
    add %g1, 64, %g1
    std %l0, [%g1 + 0]
    std %l2, [%g1 + 8]
    std %l4, [%g1 + 16]
    std %l6, [%g1 + 24]
    std %i0, [%g1 + 32]
    std %i2, [%g1 + 40]
    std %i4, [%g1 + 48]
    std %i6, [%g1 + 56]
    save
    add %g1, 64, %g1
    std %l0, [%g1 + 0]
    std %l2, [%g1 + 8]
    std %l4, [%g1 + 16]
    std %l6, [%g1 + 24]
    std %i0, [%g1 + 32]
    std %i2, [%g1 + 40]
    std %i4, [%g1 + 48]
    std %i6, [%g1 + 56]
    save
    add %g1, 64, %g1
    std %l0, [%g1 + 0]
    std %l2, [%g1 + 8]
    std %l4, [%g1 + 16]
    std %l6, [%g1 + 24]
    std %i0, [%g1 + 32]
    std %i2, [%g1 + 40]
    std %i4, [%g1 + 48]
    std %i6, [%g1 + 56]
    save
    add %g1, 64, %g1
    std %l0, [%g1 + 0]
    std %l2, [%g1 + 8]
    std %l4, [%g1 + 16]
    std %l6, [%g1 + 24]
    std %i0, [%g1 + 32]
    std %i2, [%g1 + 40]
    std %i4, [%g1 + 48]
    std %i6, [%g1 + 56]
    save
    add %g1, 64, %g1
    std %l0, [%g1 + 0]
    std %l2, [%g1 + 8]
    std %l4, [%g1 + 16]
    std %l6, [%g1 + 24]
    std %i0, [%g1 + 32]
    std %i2, [%g1 + 40]
    std %i4, [%g1 + 48]
    std %i6, [%g1 + 56]
    save
    add %g1, 64, %g1
    std %l0, [%g1 + 0]
    std %l2, [%g1 + 8]
    std %l4, [%g1 + 16]
    std %l6, [%g1 + 24]
    std %i0, [%g1 + 32]
    std %i2, [%g1 + 40]
    std %i4, [%g1 + 48]
    std %i6, [%g1 + 56]
    save
    add %g1, 64, %g1

    # restore registers
    mov %o2, %g1
    mov %o1, %wim
    nop; nop; nop

    # return
    jmp %o7+8
    nop

    .size   gc_helper_get_regs, .-gc_helper_get_regs
