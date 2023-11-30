/*
 * This file is part of the MicroPython port to MPFS platforms
 *
 * Based on LEON port originally made by George Robotics
 *
 * Copyright (c) 2023 Francescodario Cuzzocrea <bosconovic@gmail.com>
 */

#include "py/mpconfig.h"
#include "mpfs-common/mputil.h"
#include "mpfs-common/mpvmmanage.h"
#include "mpfs-common/mpsem.h"

#if MICROPY_RTEMS_ENABLE_VM_MANAGER

#if 0 // print debugging info
#define DEBUG_printf mp_printf
#else // don't print debugging info
#define DEBUG_printf(...) (void)0
#endif

// the signals that can be passed to the MicroPython VM in rtems_worker_signal
#define SIGNAL_NONE (0)
#define SIGNAL_PAUSE (1)
#define SIGNAL_RESUME (2)
#define SIGNAL_STEP (3)
#define SIGNAL_EXIT (4)
#define SIGNAL_NEW_MPY (5)

void mp_vm_worker_init(mp_state_ctx_t *worker_ctx) {
    // sem_in is for the manager to signal the worker
    int ret = mp_sem_new(&worker_ctx->vm.rtems_worker_sem_in, 0, (uint64_t)worker_ctx);

    // sem_out is for the worker to signal the manager
    ret = mp_sem_new(&worker_ctx->vm.rtems_worker_sem_out, 0, (uint64_t)worker_ctx + 1);
    (void)ret;
    // TODO check that sem_init succeeded
}

void mp_vm_worker_wait_mpy(const uint8_t **buf, size_t *len) {
    // signal to the manager that we are ready
    mp_sem_post(&MP_STATE_PORT(rtems_worker_sem_out));

    // wait for a NEW_MPY signal
    for (;;) {
        mp_sem_wait(&MP_STATE_PORT(rtems_worker_sem_in));
        if (MP_STATE_PORT(rtems_worker_signal) == SIGNAL_NEW_MPY) {
            *buf = (const uint8_t*)MP_STATE_PORT(rtems_worker_info0);
            *len = MP_STATE_PORT(rtems_worker_info1);
            return;
        }
    }
}

// Hook to execute arbitrary code within the VM loop.
void mp_vm_hook(const mp_code_state_t *code_state) {
    // check if there is a pending signal
    if (mp_sem_trywait(&MP_STATE_PORT(rtems_worker_sem_in)) != 0) {
        return;
    }

    // loop to process the signal(s)
    for (;;) {
        uint32_t note = MP_STATE_PORT(rtems_worker_signal);
        if (note == SIGNAL_PAUSE) {
            // get the current line/block/file and wait for further signals
            qstr source_file, block_name;
            size_t source_line = mp_code_state_get_line(code_state, &source_file, &block_name);
            MP_STATE_PORT(rtems_worker_info0) = source_line;
            DEBUG_printf(&mp_plat_print, "[VM pause at line %u]\n", (uint)source_line);
            mp_sem_post(&MP_STATE_PORT(rtems_worker_sem_out));
        } else if (note == SIGNAL_RESUME) {
            // resume execution
            DEBUG_printf(&mp_plat_print, "[VM resume]\n");
            mp_sem_post(&MP_STATE_PORT(rtems_worker_sem_out));
            return;
        } else if (note == SIGNAL_STEP) {
            // resume after posting the PAUSE signal to ourselves, so we stop after executing the next bytecode
            // the PAUSE will then post to sem_out to signal the VM manager that a step is completed
            MP_STATE_PORT(rtems_worker_signal) = SIGNAL_PAUSE;
            mp_sem_post(&MP_STATE_PORT(rtems_worker_sem_in));
            DEBUG_printf(&mp_plat_print, "[VM step]\n");
            return;
        } else if (note == SIGNAL_EXIT) {
            // raise a SystemExit exception to stop execution
            note = MP_STATE_PORT(rtems_worker_info0);
            DEBUG_printf(&mp_plat_print, "[VM exit with code %u]\n", (uint)note);
            MP_STATE_VM(mp_pending_exception) =
                mp_obj_new_exception_arg1(&mp_type_SystemExit, MP_OBJ_NEW_SMALL_INT(note));
            mp_sem_post(&MP_STATE_PORT(rtems_worker_sem_out));
            return;
        }
        mp_sem_wait(&MP_STATE_PORT(rtems_worker_sem_in));
    }
}

STATIC rtems_status_code mp_vm_manager_signal_and_wait(mp_state_ctx_t *worker_ctx, uint32_t signal, uint32_t timeout_ticks) {
    // signal the worker task
    worker_ctx->vm.rtems_worker_signal = signal;
    mp_sem_post(&worker_ctx->vm.rtems_worker_sem_in);

    // wait for the worker task to respond
    int ret = mp_sem_timedwait(&worker_ctx->vm.rtems_worker_sem_out, timeout_ticks);

    if (ret == 0) {
        // worker task responded in time
        return RTEMS_SUCCESSFUL;
    } else if (ret == ETIMEDOUT) {
        // timeout waiting for task to respond
        return RTEMS_TIMEOUT;
    } else {
        return RTEMS_INTERNAL_ERROR;
    }
}

rtems_status_code mp_vm_manager_start_mpy(mp_state_ctx_t *worker_ctx, const uint8_t *buf, size_t len) {
    // wait for the worker task to be ready
    mp_sem_wait(&worker_ctx->vm.rtems_worker_sem_out);

    // store the mpy pointer and length in the worker's state
    worker_ctx->vm.rtems_worker_info0 = (uint64_t)buf;
    worker_ctx->vm.rtems_worker_info1 = len;

    // signal that there is a new mpy
    worker_ctx->vm.rtems_worker_signal = SIGNAL_NEW_MPY;
    mp_sem_post(&worker_ctx->vm.rtems_worker_sem_in);

    return RTEMS_SUCCESSFUL;
}

rtems_status_code mp_vm_manager_pause(mp_state_ctx_t *worker_ctx, uint32_t timeout_ticks, uint32_t *source_line) {
    rtems_status_code status;

    // signal and wait for script to pause
    status = mp_vm_manager_signal_and_wait(worker_ctx, SIGNAL_PAUSE, timeout_ticks);
    if (status != RTEMS_SUCCESSFUL) {
        return status;
    }

    // get line number that the script paused at
    *source_line = worker_ctx->vm.rtems_worker_info0;

    return RTEMS_SUCCESSFUL;
}

rtems_status_code mp_vm_manager_resume(mp_state_ctx_t *worker_ctx, uint32_t timeout_ticks) {
    // signal and wait for script to resume
    return mp_vm_manager_signal_and_wait(worker_ctx, SIGNAL_RESUME, timeout_ticks);
}

rtems_status_code mp_vm_manager_step(mp_state_ctx_t *worker_ctx, uint32_t timeout_ticks, uint32_t *source_line) {
    rtems_status_code status;

    // signal and wait for script to pause again
    status = mp_vm_manager_signal_and_wait(worker_ctx, SIGNAL_STEP, timeout_ticks);
    if (status != RTEMS_SUCCESSFUL) {
        return status;
    }

    // get line number that the script paused at
    *source_line = worker_ctx->vm.rtems_worker_info0;

    return RTEMS_SUCCESSFUL;
}

rtems_status_code mp_vm_manager_exit(mp_state_ctx_t *worker_ctx, uint32_t timeout_ticks, uint32_t exit_code) {
    // set the exit code
    worker_ctx->vm.rtems_worker_info0 = exit_code;

    // signal and wait for script to exit
    return mp_vm_manager_signal_and_wait(worker_ctx, SIGNAL_EXIT, timeout_ticks);
}

#endif // MICROPY_RTEMS_ENABLE_VM_MANAGER
