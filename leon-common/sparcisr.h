/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2017 George Robotics Limited
 */

#pragma once

// This function is needed to install the "ta 3" window flush handler.
void sparc_install_ta_3_window_flush_isr(void);
