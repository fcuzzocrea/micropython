/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2015-2022 George Robotics Limited
 *
 * Provided to the European Space Agency as part of the project "Evolutions
 * of MicroPython for LEON", contract number 4000137198/22/NL/MGu/kk.
 */

#pragma once

#include <rtems.h>
#include "rtems_config.h"

rtems_task Init(rtems_task_argument ignored);

#if MICROPY_RTEMS_USE_TASK_CONSTRUCT
void mp_rtems_task_config(rtems_task_config *config, rtems_name name, void *storage_area, size_t storage_size);
#endif
