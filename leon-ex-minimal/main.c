/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2015-2016 George Robotics Limited
 *
 * Provided to the European Space Agency as part of the project "Porting of
 * MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
 */

#include <stdio.h>
#include <rtems.h>

#define CONFIGURE_INIT
#define CONFIGURE_INIT_TASK_ENTRY_POINT Init
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_MAXIMUM_TASKS (4)
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_EXTRA_TASK_STACKS (3 * RTEMS_MINIMUM_STACK_SIZE)

rtems_task Init(rtems_task_argument argument);
rtems_task mp_manager_task(rtems_task_argument unused);
rtems_task mp_worker_task(rtems_task_argument unused);

#include <rtems/confdefs.h>

#define MICROPY_RTEMS_TASK_ATTRIBUTES (RTEMS_APPLICATION_TASK | RTEMS_FLOATING_POINT)
#define MICROPY_RTEMS_STACK_SIZE (RTEMS_MINIMUM_STACK_SIZE * 2)
#define MICROPY_RTEMS_HEAP_SIZE (16 * 1024)

#ifndef MICROPY_RTEMS_NUM_TASKS
#define MICROPY_RTEMS_NUM_TASKS (1)
#endif

/******************************************************************************/
// RTEMS initialisation task
// this task runs at highest priority and is non-preemptive

rtems_task Init(rtems_task_argument ignored) {
    // set the time
    rtems_time_of_day time;
    time.year = 2016;
    time.month = 1;
    time.day = 1;
    time.hour = 0;
    time.minute = 0;
    time.second = 0;
    time.ticks = 0;
    rtems_clock_set(&time);

    // initialise the message queue subsystem
    #if RTEMS_4_8
    _Message_queue_Manager_initialization(4);
    #else
    _Message_queue_Manager_initialization();
    #endif

    // initialise the timer subsystem
    #if RTEMS_4_8
    _Timer_Manager_initialization(2);
    #else
    _Timer_Manager_initialization();
    #endif

    // start the manager task to do the rest of the work
    rtems_name task_name = rtems_build_name('M', 'P', 'M', 'A');
    rtems_id task_id;
    rtems_status_code status;
    status = rtems_task_create(
        task_name, 1, RTEMS_MINIMUM_STACK_SIZE, RTEMS_DEFAULT_MODES,
        MICROPY_RTEMS_TASK_ATTRIBUTES, &task_id
    );
    status = rtems_task_start(task_id, mp_manager_task, 0);
    (void)status; // status not checked
    rtems_task_delete(RTEMS_SELF);
}

/******************************************************************************/
// MicroPython manager task

#include "py/mphal.h"

// this function is used as a hook to set a breakpoint to terminate emu
void emu_terminate(void) {
    printf("emu_terminate\n");
}

rtems_task mp_manager_task(rtems_task_argument ignored) {
    printf("\nMicroPython manager task started\n");

    rtems_name task_name[MICROPY_RTEMS_NUM_TASKS];
    rtems_id task_id[MICROPY_RTEMS_NUM_TASKS];

    // spawn all worker tasks
    for (int i = 0; i < MICROPY_RTEMS_NUM_TASKS; ++i) {
        rtems_status_code status;
        task_name[i] = rtems_build_name('M', 'P', '0', '0' + i);
        status = rtems_task_create(
            task_name[i], 1, MICROPY_RTEMS_STACK_SIZE, RTEMS_DEFAULT_MODES,
            MICROPY_RTEMS_TASK_ATTRIBUTES, &task_id[i]
        );
        status = rtems_task_start(task_id[i], mp_worker_task, i);
        (void)status; // status not checked
    }

    // wait for termination
    for (;;) {
        rtems_task_wake_after(100);
    }
}

/******************************************************************************/
// MicroPython worker task

#include "py/runtime.h"
#include "py/gc.h"
#include "py/stackctrl.h"
#include "mputil.h"

// include the precompiled bytecode (generated by external tools)
#include "build/scripts.h"

static mp_state_ctx_t mp_state_ctx[MICROPY_RTEMS_NUM_TASKS];
static byte mp_heap[MICROPY_RTEMS_NUM_TASKS * MICROPY_RTEMS_HEAP_SIZE];

rtems_task mp_worker_task(rtems_task_argument task_index) {
    // set the MicroPython context for this task
    _Thread_Executing->Start.numeric_argument = (uint32_t)&mp_state_ctx[task_index];

    // set value for rtems.script_id() function
    MP_STATE_PORT(rtems_script_id) = mp_obj_new_int(task_index);

    // get a pointer to the heap for this task
    byte *heap_start = &mp_heap[task_index * MICROPY_RTEMS_HEAP_SIZE];

    // loop and execute all the scripts
    for (int script_num = 0; script_num < mpy_script_num; ++script_num) {
        // initialise the stack limit checking
        mp_stack_ctrl_init();
        mp_stack_set_limit(MICROPY_RTEMS_STACK_SIZE - 2048);

        // initialise the heap
        gc_init(heap_start, heap_start + MICROPY_RTEMS_HEAP_SIZE);

        // initialise the MicroPython runtime
        mp_init();

        // register the last script for importing
        mp_mpy_modules_init();
        mp_mpy_modules_register(mpy_script_data[mpy_script_num - 1], mpy_script_len[mpy_script_num - 1]);

        // execute the bytecode
        uint32_t retval = mp_exec_mpy(mpy_script_data[script_num], mpy_script_len[script_num]);

        // check the return value
        if (retval != 0) {
            mp_printf(&mp_plat_print, "retval = %u\n", (uint)retval);
        }

        // deinitialise the MicroPython runtime
        mp_deinit();
    }

    // terminate emulator
    emu_terminate();
    rtems_task_delete(RTEMS_SELF);
}
