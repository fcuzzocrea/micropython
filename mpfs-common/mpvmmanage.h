/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2015-2016 George Robotics Limited
 *
 * Provided to the European Space Agency as part of the project "Porting of
 * MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
 */

#pragma once

#include <rtems.h>

// This function is to be used by the VM manager task to initialise a given
// VM worker task, specified by its MicroPython context.
struct _mp_state_ctx_t;
void mp_vm_worker_init(struct _mp_state_ctx_t *worker_ctx);

// This function is to be used by VM worker tasks, which take scripts and
// execute them.  It will block until some MPY code is available.
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
rtems_status_code mp_vm_manager_start_mpy(struct _mp_state_ctx_t *worker_ctx, const uint8_t* buf, size_t len);
rtems_status_code mp_vm_manager_pause(struct _mp_state_ctx_t *worker_ctx, uint32_t timeout_ticks, uint32_t *source_line);
rtems_status_code mp_vm_manager_resume(struct _mp_state_ctx_t *worker_ctx, uint32_t timeout_ticks);
rtems_status_code mp_vm_manager_step(struct _mp_state_ctx_t *worker_ctx, uint32_t timeout_ticks, uint32_t *source_line);
rtems_status_code mp_vm_manager_exit(struct _mp_state_ctx_t *worker_ctx, uint32_t timeout_ticks, uint32_t exit_code);
