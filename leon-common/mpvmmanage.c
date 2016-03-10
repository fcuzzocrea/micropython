/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2015-2016 George Robotics Limited
 *
 * Provided to the European Space Agency as part of the project "Porting of
 * MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
 */

#include "py/mpconfig.h"
#include "leon-common/mputil.h"
#include "leon-common/mpvmmanage.h"

#if 0 // print debugging info
#define DEBUG_printf printf
#else // don't print debugging info
#define DEBUG_printf(...) (void)0
#endif

// the rtems notepad entries to use to do the signalling between tasks
#define NOTEPAD_STATE (RTEMS_NOTEPAD_0)
#define NOTEPAD_INFO (RTEMS_NOTEPAD_1)

// the number of rtems ticks to use when polling the notepad for a change
#define TICKS_POLL (10)

// the signals that can be passed to the MicroPython VM in the notepad
#define SIGNAL_NONE (0)
#define SIGNAL_PAUSE (1)
#define SIGNAL_RESUME (2)
#define SIGNAL_STEP (3)
#define SIGNAL_EXIT (4)

// Hook to execute arbitrary code within the VM loop.
void mp_vm_hook(const mp_code_state *code_state) {
    // check if there is a pending signal
    uint32_t note;
    rtems_task_get_note(RTEMS_SELF, NOTEPAD_STATE, &note);
    if (note == SIGNAL_NONE) {
        return;
    }

    // loop to process the signal
    for (;;) {
        uint32_t note;
        rtems_task_get_note(RTEMS_SELF, NOTEPAD_STATE, &note);
        if (note == SIGNAL_PAUSE) {
            qstr source_file, block_name;
            size_t source_line = mp_code_state_get_line(code_state, &source_file, &block_name);
            rtems_task_set_note(RTEMS_SELF, NOTEPAD_INFO, source_line);
            rtems_task_set_note(RTEMS_SELF, NOTEPAD_STATE, SIGNAL_NONE);
            DEBUG_printf(&mp_plat_print, "[VM pause at line %u]\n", (uint)source_line);
        } else if (note == SIGNAL_RESUME) {
            rtems_task_set_note(RTEMS_SELF, NOTEPAD_STATE, SIGNAL_NONE);
            DEBUG_printf(&mp_plat_print, "[VM resume]\n");
            return;
        } else if (note == SIGNAL_STEP) {
            rtems_task_set_note(RTEMS_SELF, NOTEPAD_STATE, SIGNAL_PAUSE);
            DEBUG_printf(&mp_plat_print, "[VM step]\n");
            return;
        } else if (note == SIGNAL_EXIT) {
            rtems_task_get_note(RTEMS_SELF, NOTEPAD_INFO, &note);
            rtems_task_set_note(RTEMS_SELF, NOTEPAD_STATE, SIGNAL_NONE);
            DEBUG_printf(&mp_plat_print, "[VM exit with code %u]\n", (uint)note);
            MP_STATE_VM(mp_pending_exception) =
                mp_obj_new_exception_arg1(&mp_type_SystemExit, MP_OBJ_NEW_SMALL_INT(note));
            return;
        }
        rtems_task_wake_after(TICKS_POLL);
    }
}

void mp_vm_manager_init(rtems_id task_id) {
    rtems_task_set_note(task_id, NOTEPAD_STATE, SIGNAL_NONE);
}

STATIC rtems_status_code mp_vm_manager_signal_and_wait(rtems_id task_id, uint32_t signal, rtems_interval ticks_timeout) {
    rtems_status_code status;

    // signal task
    status = rtems_task_set_note(task_id, NOTEPAD_STATE, signal);
    if (status != RTEMS_SUCCESSFUL) {
        return status;
    }

    // wait for task to respond
    for (rtems_interval ticks = 0; ticks <= ticks_timeout; ticks += TICKS_POLL) {
        status = rtems_task_wake_after(TICKS_POLL);
        if (status != RTEMS_SUCCESSFUL) {
            return status;
        }
        uint32_t note;
        status = rtems_task_get_note(task_id, NOTEPAD_STATE, &note);
        if (status != RTEMS_SUCCESSFUL) {
            return status;
        }
        if (note != signal) {
            return RTEMS_SUCCESSFUL;
        }
    }

    // timeout waiting for task to respond
    return RTEMS_TIMEOUT;
}

rtems_status_code mp_vm_manager_pause(rtems_id task_id, rtems_interval ticks_timeout, uint32_t *source_line) {
    rtems_status_code status; 

    // signal and wait for script to pause
    status = mp_vm_manager_signal_and_wait(task_id, SIGNAL_PAUSE, ticks_timeout);
    if (status != RTEMS_SUCCESSFUL) {
        return status;
    }

    // get line number that the script paused at
    return rtems_task_get_note(task_id, NOTEPAD_INFO, source_line);
}

rtems_status_code mp_vm_manager_resume(rtems_id task_id, rtems_interval ticks_timeout) {
    rtems_status_code status; 

    // signal and wait for script to resume
    status = mp_vm_manager_signal_and_wait(task_id, SIGNAL_RESUME, ticks_timeout);
    return status;
}

rtems_status_code mp_vm_manager_step(rtems_id task_id, rtems_interval ticks_timeout, uint32_t *source_line) {
    rtems_status_code status; 

    // signal and wait for script to pause again
    status = mp_vm_manager_signal_and_wait(task_id, SIGNAL_STEP, ticks_timeout);
    if (status != RTEMS_SUCCESSFUL) {
        return status;
    }

    // get line number that the script paused at
    return rtems_task_get_note(task_id, NOTEPAD_INFO, source_line);
}

rtems_status_code mp_vm_manager_exit(rtems_id task_id, rtems_interval ticks_timeout, uint32_t exit_code) {
    rtems_status_code status; 

    // set the exit code
    status = rtems_task_set_note(task_id, NOTEPAD_INFO, exit_code);
    if (status != RTEMS_SUCCESSFUL) {
        return status;
    }

    // signal and wait for script to exit
    status = mp_vm_manager_signal_and_wait(task_id, SIGNAL_EXIT, ticks_timeout);
    return status;
}
