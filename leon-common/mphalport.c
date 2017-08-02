/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2015-2016 George Robotics Limited
 *
 * Provided to the European Space Agency as part of the project "Porting of
 * MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
 */

#include "py/mpconfig.h"
#include "py/mphal.h"

// Send string of given length
void mp_hal_stdout_tx_strn(const char *str, size_t len) {
    extern void output_a_character_to_leon2_port0(unsigned char c);
    while (len--) {
        output_a_character_to_leon2_port0(*str++);
    }
}

#define MP_HAL_STDOUT_HEXLIFY_MAX_N (32)
static int mp_hal_stdout_hexlify = 0;
static int mp_hal_stdout_hexlify_n = 0;

void mp_hal_stdout_enable_hexlify(void) {
    if (!mp_hal_stdout_hexlify) {
        mp_hal_stdout_tx_strn("\nHEX ", 5);
        mp_hal_stdout_hexlify = 1;
        mp_hal_stdout_hexlify_n = 0;
    }
}
void mp_hal_stdout_disable_hexlify(void) {
    if (mp_hal_stdout_hexlify) {
        mp_hal_stdout_tx_strn("\n", 1);
        mp_hal_stdout_hexlify = 0;
    }
}

// Send "cooked" string of length, where every occurance of
// LF character is replaced with CR LF.
void mp_hal_stdout_tx_strn_cooked(const char *str, size_t len) {
    // leon2-emu turns multiple new lines into a single new line
    // to get around this we prepend all lines with ': ' and then
    // cut them out with a post processing tool

    /*
    static int start_of_line = 1;

    while (len--) {
        if (start_of_line) {
            mp_hal_stdout_tx_strn(": ", 2);
            start_of_line = 0;
        }
        if (*str == '\n') {
            start_of_line = 1;
        }
        mp_hal_stdout_tx_strn(str++, 1);
    }
    */

    if (mp_hal_stdout_hexlify) {
        static const char to_hex[16] = "0123456789abcdef";
        while (len--) {
            char buf[2];
            if (mp_hal_stdout_hexlify_n >= MP_HAL_STDOUT_HEXLIFY_MAX_N) {
                mp_hal_stdout_tx_strn("\nHEX ", 5);
                mp_hal_stdout_hexlify_n = 0;
            }
            buf[0] = to_hex[(*str >> 4) & 0xf];
            buf[1] = to_hex[(*str) & 0xf];
            mp_hal_stdout_tx_strn(buf, 2);
            ++str;
            ++mp_hal_stdout_hexlify_n;
        }
    } else {
        mp_hal_stdout_tx_strn(str, len);
    }
}

// Send zero-terminated string
void mp_hal_stdout_tx_str(const char *str) {
    mp_hal_stdout_tx_strn(str, strlen(str));
}
