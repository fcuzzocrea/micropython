/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2018 George Robotics Limited
 */

#pragma once

#include <stdarg.h>

int leon_printf(const char *fmt, ...);
int leon_vprintf(const char *fmt, va_list args);
