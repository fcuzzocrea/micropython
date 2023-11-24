/*
 * This file is part of the MicroPython port to MPFS platforms
 *
 * Based on LEON port originally made by George Robotics
 *
 * Copyright (c) 2023 Francescodario Cuzzocrea <bosconovic@gmail.com>
 */

#pragma once

#include <rtems.h>

extern const mp_obj_module_t mp_module_rtems_task;
extern const mp_obj_module_t mp_module_rtems_queue;
extern const mp_obj_module_t mp_module_rtems_sem;

void mod_rtems_status_code_check(rtems_status_code status);
rtems_name mod_rtems_name_from_obj(mp_obj_t name_in);
