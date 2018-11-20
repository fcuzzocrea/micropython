/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2017 George Robotics Limited
 */

#include <stdint.h>
#include "leon-common/sparcisr.h"

// This function retrieves the value of the Trap Base Register, to be used
// to install a window flushing trap handler next.
static void TRAP_read_TBR(volatile uint32_t* const tbrPtr) {
    asm("mov %0, %%o1" : : "r" (tbrPtr) : "%o1");
    asm("rd %%tbr, %%o0" : : : "%o0");
    asm("st %o0, [%o1]");
}

// This function installs the "ta 3" ISR which handles window flushing.
// It is only needed for Edisoft RTEMS 4.8 because the trap handler is not
// included in that version, but it is needed by MicroPython to implement
// setjmp/longjmp calls for exception handling.
void sparc_install_ta_3_window_flush_isr(void) {
    extern int sparc_window_flush_trap_handler(void);
    volatile uint32_t tbr = 0;
    TRAP_read_TBR(&tbr);
    tbr &= 0xfffff000;
    tbr |= 0x830;
    uint32_t *m = (uint32_t*)tbr;
    uint32_t addr = (uint32_t)sparc_window_flush_trap_handler;
    m[0] = 0xa1480000; // rd %psr, %l0
    m[1] = 0x29100000 | (addr >> 10); // sethi %hi(addr), %l4
    m[2] = 0x81c52000 | (addr & 0x3ff); // jmp %l4 + (addr & 0x3ff)
    m[3] = 0xa6102083; // mov 0x83, %l3
}
