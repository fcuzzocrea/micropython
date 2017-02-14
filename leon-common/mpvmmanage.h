/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2015-2016 George Robotics Limited
 *
 * Provided to the European Space Agency as part of the project "Porting of
 * MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
 */

#pragma once

#include <rtems.h>

// These functions are to be used by VM worker tasks, which take scripts
// and execute them.
void mp_vm_worker_init(void);
void mp_vm_worker_wait_mpy(const uint8_t **buf, size_t *len);

// This function is for the worker VM to call periodically to check if there
// are any outstanding requests (eg pause, resume, exit).
struct _mp_code_state_t;
void mp_vm_hook(const struct _mp_code_state_t *code_state);

// These functions are for use by the VM manager task to control VM worker
// tasks.  The "task_id" parameter is the RTEMS id of the target VM worker
// task.  A manager can control one or more worker tasks.
// Note: these functions are not thread safe and assume that each VM worker
// task has a unique VM manager task associated with it.  Making them thread
// safe would require the use of a semaphore.
rtems_status_code mp_vm_manager_start_mpy(rtems_id task_id, const uint8_t* buf, size_t len);
rtems_status_code mp_vm_manager_pause(rtems_id task_id, rtems_interval ticks_timeout, uint32_t *source_line);
rtems_status_code mp_vm_manager_resume(rtems_id task_id, rtems_interval ticks_timeout);
rtems_status_code mp_vm_manager_step(rtems_id task_id, rtems_interval ticks_timeout, uint32_t *source_line);
rtems_status_code mp_vm_manager_exit(rtems_id task_id, rtems_interval ticks_timeout, uint32_t exit_code);
