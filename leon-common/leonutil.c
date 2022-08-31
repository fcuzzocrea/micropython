/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2022 George Robotics Limited
 *
 * Provided to the European Space Agency as part of the project "Evolutions
 * of MicroPython for LEON", contract number 4000137198/22/NL/MGu/kk.
 */

#include <rtems.h>
#include "rtems_config.h"
#include "leon-common/leonprintf.h"
#include "leon-common/leonutil.h"

// this function is used as a hook to set a breakpoint to terminate emu
void leon_emu_terminate(void) {
    leon_printf("emu_terminate\n");
    #if MICROPY_RTEMS_USE_FATAL
    rtems_fatal(RTEMS_FATAL_SOURCE_EXIT, 0);
    #endif
}
