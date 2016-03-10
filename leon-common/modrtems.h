/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2015-2016 George Robotics Limited
 *
 * Provided to the European Space Agency as part of the project "Porting of
 * MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
 */

#pragma once

extern const mp_obj_module_t mp_module_rtems_task;
extern const mp_obj_module_t mp_module_rtems_queue;
extern const mp_obj_module_t mp_module_rtems_timer;

void mod_rtems_status_code_check(rtems_status_code status);
rtems_name mod_rtems_name_from_obj(mp_obj_t name_in);
