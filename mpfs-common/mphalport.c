/*
 * This file is part of the MicroPython port to MPFS platforms
 *
 * Based on LEON port originally made by George Robotics
 *
 * Copyright (c) 2023 Francescodario Cuzzocrea <bosconovic@gmail.com>
 */

#include <stdio.h>
#include <unistd.h>
#include "py/mpconfig.h"
#include "py/mphal.h"

// Send string of given length
void mp_hal_stdout_tx_strn(const char *str, size_t len) {
    ssize_t r = write(STDOUT_FILENO, str, len);
    (void)r;
}

// Send "cooked" string of length, where every occurance of
// LF character is replaced with CR LF.
void mp_hal_stdout_tx_strn_cooked(const char *str, size_t len) {
    mp_hal_stdout_tx_strn(str, len);
}
