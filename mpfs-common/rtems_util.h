/*
 * This file is part of the MicroPython port to MPFS platforms
 *
 * Based on LEON port originally made by George Robotics
 *
 * Copyright (c) 2023 Francescodario Cuzzocrea <bosconovic@gmail.com>
 */

#pragma once

#include <rtems.h>
#include "rtems_config.h"

rtems_task Init(rtems_task_argument ignored);

#if MICROPY_RTEMS_USE_TASK_CONSTRUCT
void mp_rtems_task_config(rtems_task_config *config, rtems_name name, void *storage_area, size_t storage_size);
#endif
