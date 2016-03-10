/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2015-2016 George Robotics Limited
 *
 * Provided to the European Space Agency as part of the project "Porting of
 * MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
 */

#pragma once

#include <rtems.h>

void mp_vm_manager_init(rtems_id task_id);
rtems_status_code mp_vm_manager_pause(rtems_id task_id, rtems_interval ticks_timeout, uint32_t *source_line);
rtems_status_code mp_vm_manager_resume(rtems_id task_id, rtems_interval ticks_timeout);
rtems_status_code mp_vm_manager_step(rtems_id task_id, rtems_interval ticks_timeout, uint32_t *source_line);
rtems_status_code mp_vm_manager_exit(rtems_id task_id, rtems_interval ticks_timeout, uint32_t exit_code);
