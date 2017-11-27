# This file is part of the MicroPython port to LEON platforms
# Copyright (c) 2017 George Robotics Limited
#
# This file provides a custom setjmp/longjmp implementation for LEON2/SPARC with
# 8 register windows.
# The size of the jump buffer should be 72 bytes, to store 18 registers.

    .file "sparcjmp.s"
    .section ".text"
    .align 4

    .global sparc_setjmp
    .type sparc_setjmp, #function

    .global sparc_longjmp
    .type sparc_longjmp, #function

    .global sparc_window_flush_trap_handler
    .type sparc_window_flush_trap_handler, #function

# sparc_setjmp(%o0=env)
sparc_setjmp:
    # save the sp (%o6) and return pointer (%o7)
    st  %sp, [%o0 + 0x00]
    st  %o7, [%o0 + 0x04]

    # save the local and input registers
    st  %l0, [%o0 + 0x08]
    st  %l1, [%o0 + 0x0c]
    st  %l2, [%o0 + 0x10]
    st  %l3, [%o0 + 0x14]
    st  %l4, [%o0 + 0x18]
    st  %l5, [%o0 + 0x1c]
    st  %l6, [%o0 + 0x20]
    st  %l7, [%o0 + 0x24]
    st  %i0, [%o0 + 0x28]
    st  %i1, [%o0 + 0x2c]
    st  %i2, [%o0 + 0x30]
    st  %i3, [%o0 + 0x34]
    st  %i4, [%o0 + 0x38]
    st  %i5, [%o0 + 0x3c]
    st  %i6, [%o0 + 0x40]
    st  %i7, [%o0 + 0x44]

    # return 0
    retl
    mov  %g0, %o0

    .size   sparc_setjmp, .-sparc_setjmp

# sparc_longjmp(env, val):
sparc_longjmp:
    ta 3
    addcc  %o1, %g0, %g6
    be,a   .is_zero
    mov  1, %g6
.is_zero:

    # restore the local and input registers (some overwrite the above)
    ld  [%o0 + 0x08], %l0
    ld  [%o0 + 0x0c], %l1
    ld  [%o0 + 0x10], %l2
    ld  [%o0 + 0x14], %l3
    ld  [%o0 + 0x18], %l4
    ld  [%o0 + 0x1c], %l5
    ld  [%o0 + 0x20], %l6
    ld  [%o0 + 0x24], %l7
    ld  [%o0 + 0x28], %i0
    ld  [%o0 + 0x2c], %i1
    ld  [%o0 + 0x30], %i2
    ld  [%o0 + 0x34], %i3
    ld  [%o0 + 0x38], %i4
    ld  [%o0 + 0x3c], %i5
    ld  [%o0 + 0x40], %i6
    ld  [%o0 + 0x44], %i7

    # restore the sp (%o6) and return pointer (%o7)
    ld  [%o0 + 0x00], %sp
    ld  [%o0 + 0x04], %o7

    # return the passed-in "val"
    retl
    mov  %g6, %o0

    .size   sparc_longjmp, .-sparc_longjmp

# from the ta 3, %l0 should contain the PSR
# locals can be clobbered because they will be restored
sparc_window_flush_trap_handler:
    # save global registers
    mov  %g1, %l3
    mov  %g2, %l4
    mov  %g3, %l5
    mov  %g4, %l6
    mov  %g5, %l7

    mov  %l0, %g1
    rd  %wim, %g2

    # %g3 = CWP (hard-coded for 8 windows)
    and  %l0, 7, %g3

    # %g5 = (CWP + 1) % NWINDOWS
    add  %g3, 1, %g5
    and  %g5, 7, %g5

    # %g4 = 1 << ((CWP + 1) % NWINDOWS)
    mov  1, %g4
    sll  %g4, %g5, %g4

.save_frame_loop:
    # restore will increment CWP in the PSR
    restore
    std  %l0, [ %sp ]
    std  %l2, [ %sp + 8 ]
    std  %l4, [ %sp + 0x10 ]
    std  %l6, [ %sp + 0x18 ]
    std  %i0, [ %sp + 0x20 ]
    std  %i2, [ %sp + 0x28 ]
    std  %i4, [ %sp + 0x30 ]
    std  %i6, [ %sp + 0x38 ]

    # %g4 = (%g4 >> 7) | (%g4 << 1) [equiv: rol %g4 by 1]
    sll  %g4, 1, %g5
    srl  %g4, 7, %g4
    or  %g4, %g5, %g4

    # if (%g4 & %wim == 0) goto .save_frame_loop;
    btst  %g4, %g2
    be  .save_frame_loop
    nop

    # %g3 = (CWP + 2) % NWINDOWS
    add  %g3, 2, %g3
    and  %g3, 7, %g3

    # %wim = 1 << ((CWP + 2) % NWINDOWS)
    mov  1, %g4
    sll  %g4, %g3, %g4
    mov  %g4, %wim

    # restore PSR (and interrupt mask PIL)
    wr  %g1, %psr
    nop
    nop
    nop

    # restore global registers
    mov  %l3, %g1
    mov  %l4, %g2
    mov  %l5, %g3
    mov  %l6, %g4
    mov  %l7, %g5

    # return
    jmp  %l2
    rett  %l2 + 4

    .size   sparc_window_flush_trap_handler, .-sparc_window_flush_trap_handler
