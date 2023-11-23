/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2018 George Robotics Limited
 */

#include "leon-common/leonprintf.h"
#include "rtems_config.h"

#if RTEMS_4_8_EDISOFT
// RTEMS 4.8 Edisoft has a custom output function provided by leon2serial.c.
extern void output_a_character_to_leon2_port0(unsigned char c);
#elif RTEMS_4
// RTEMS 4.x versions (non-Edisoft) use this function for output.
extern void console_outbyte_polled(int port, unsigned char ch);
#else
// RTEMS 5 and RTEMS 6 have an rtems_putc function defined in this header file.
#include <rtems/bspIo.h>
#endif

void leon_putc(char c) {
    #if RTEMS_4_8_EDISOFT
    output_a_character_to_leon2_port0(c);
    #elif RTEMS_4
    console_outbyte_polled(0, c);
    #else
    rtems_putc(c);
    #endif
}

int leon_printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = leon_vprintf(fmt, ap);
    va_end(ap);
    return ret;
}

int leon_vprintf(const char *fmt, va_list args) {
    int chrs = 0;
    while (*fmt != '\0') {
        if (*fmt == '%') {
            ++fmt;
            if (*fmt == 'u' || *fmt == 'x') {
                unsigned int base = *fmt == 'u' ? 10 : 16;
                ++fmt;
                unsigned int i = va_arg(args, unsigned int);
                char buf[32];
                char *b = &buf[0];
                if (i == 0) {
                    *b++ = '0';
                } else {
                    while (i && b < &buf[sizeof(buf)]) {
                        unsigned int digit = i % base;
                        if (digit <= 9) {
                            *b++ = '0' + digit;
                        } else {
                            *b++ = 'a' + digit - 10;
                        }
                        i /= base;
                    }
                }
                while (b > &buf[0]) {
                    leon_putc(*--b);
                    chrs += 1;
                }
            } else if (*fmt != '\0') {
                // print any other format specifiers (handles %% correctly)
                leon_putc(*fmt++);
                chrs += 1;
            }
        } else {
            // print non-format character
            leon_putc(*fmt++);
            chrs += 1;
        }
    }
    return chrs;
}