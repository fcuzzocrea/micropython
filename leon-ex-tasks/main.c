/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2015-2016 George Robotics Limited
 *
 * Provided to the European Space Agency as part of the project "Porting of
 * MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
 */

#include <stdio.h>
#include <rtems.h>
#include "rtems_config.h"
#include "leon-common/rtems_util.h"
#include "leon-common/sparcisr.h"

rtems_task mp_manager_task(rtems_task_argument unused);
rtems_task mp_worker_task(rtems_task_argument unused);

/******************************************************************************/
// RTEMS initialisation task
// this task runs at highest priority and is non-preemptive

#if MICROPY_RTEMS_USE_TASK_CONSTRUCT
RTEMS_ALIGNED(RTEMS_TASK_STORAGE_ALIGNMENT)
static char mpma_task_storage[TASK_STORAGE_SIZE];
static rtems_task_config mpma_task_config;
#endif

rtems_task Init(rtems_task_argument ignored) {
    sparc_install_ta_3_window_flush_isr();

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
    #elif RTEMS_4
    _Message_queue_Manager_initialization();
    #endif

    // initialise the timer subsystem
    #if RTEMS_4_8
    _Timer_Manager_initialization(2);
    #elif RTEMS_4
    _Timer_Manager_initialization();
    #endif

    // start the manager task to do the rest of the work
    rtems_name task_name = rtems_build_name('M', 'P', 'M', 'A');
    rtems_id task_id;
    rtems_status_code status;
    #if MICROPY_RTEMS_USE_TASK_CONSTRUCT
    mp_rtems_task_config(&mpma_task_config, task_name, mpma_task_storage, sizeof(mpma_task_storage));
    status = rtems_task_construct(&mpma_task_config, &task_id);
    #else
    status = rtems_task_create(
        task_name, 1, RTEMS_MINIMUM_STACK_SIZE, RTEMS_DEFAULT_MODES,
        MICROPY_RTEMS_TASK_ATTRIBUTES, &task_id
    );
    #endif
    if (status != RTEMS_SUCCESSFUL) {
        return;
    }
    status = rtems_task_start(task_id, mp_manager_task, 0);
    if (status != RTEMS_SUCCESSFUL) {
        return;
    }
    rtems_task_delete(RTEMS_SELF);
}

/******************************************************************************/
// MicroPython manager task

#include "leon-common/leonprintf.h"
#include "leon-common/leonutil.h"

#if MICROPY_RTEMS_USE_TASK_CONSTRUCT
RTEMS_ALIGNED(RTEMS_TASK_STORAGE_ALIGNMENT)
static char mp_task_storage[MICROPY_RTEMS_NUM_TASKS][TASK_STORAGE_SIZE];
static rtems_task_config mp_task_config[MICROPY_RTEMS_NUM_TASKS];
#endif

rtems_task mp_manager_task(rtems_task_argument ignored) {
    leon_printf("\nMicroPython manager task started\n");

    rtems_id task_id[MICROPY_RTEMS_NUM_TASKS];

    // spawn all worker tasks
    for (int i = 0; i < MICROPY_RTEMS_NUM_TASKS; ++i) {
        rtems_status_code status;
        rtems_name task_name = rtems_build_name('M', 'P', '0', '0' + i);
        #if MICROPY_RTEMS_USE_TASK_CONSTRUCT
        rtems_task_config *config = &mp_task_config[i];
        mp_rtems_task_config(config, task_name, mp_task_storage[i], sizeof(mp_task_storage[i]));
        status = rtems_task_construct(config, &task_id[i]);
        #else
        status = rtems_task_create(
            task_name, 1, MICROPY_RTEMS_STACK_SIZE, RTEMS_DEFAULT_MODES,
            MICROPY_RTEMS_TASK_ATTRIBUTES, &task_id[i]
        );
        #endif
        status = rtems_task_start(task_id[i], mp_worker_task, i);
        (void)status; // status not checked
    }

    // wait for all worker tasks to complete
    for (;;) {
        rtems_task_wake_after(200);
        int num_tasks_complete = 0;
        for (int i = 0; i < MICROPY_RTEMS_NUM_TASKS; ++i) {
            if (rtems_task_is_suspended(task_id[i]) == RTEMS_ALREADY_SUSPENDED) {
                num_tasks_complete += 1;
            }
        }
        if (num_tasks_complete == MICROPY_RTEMS_NUM_TASKS) {
            leon_emu_terminate();
        }
    }
}

/******************************************************************************/
// MicroPython worker task

#include "py/runtime.h"
#include "py/gc.h"
#include "py/stackctrl.h"
#include "mputil.h"

// Location in RAM for external scripts.  Allow 64kiB per script.
#define MPY_MEM_BASE   (MICROPY_RTEMS_MPY_MEM_BASE)
#define MPY_MEM_STRIDE (0x00010000)

static mp_state_ctx_t mp_state_ctx[MICROPY_RTEMS_NUM_TASKS];
static byte mp_heap[MICROPY_RTEMS_NUM_TASKS * MICROPY_RTEMS_HEAP_SIZE];

rtems_task mp_worker_task(rtems_task_argument task_index) {
    // set the MicroPython context for this task
    mp_state_ptr_set(&mp_state_ctx[task_index]);

    // set value for rtems.script_id() function
    MP_STATE_PORT(rtems_script_id) = mp_obj_new_int(task_index);

    // initialise the stack limit checking
    mp_stack_ctrl_init();
    mp_stack_set_limit(MICROPY_RTEMS_STACK_SIZE - 2048);

    // initialise the heap
    byte *heap_start = &mp_heap[task_index * MICROPY_RTEMS_HEAP_SIZE];
    gc_init(heap_start, heap_start + MICROPY_RTEMS_HEAP_SIZE);

    // initialise the MicroPython runtime
    mp_init();

    // get the precompiled bytecode from the fixed address in RAM
    const void *mpy_base = (const void*)(MPY_MEM_BASE + MPY_MEM_STRIDE * task_index);
    size_t mpy_len = *(const uint32_t*)mpy_base;
    const byte *mpy_data = (const byte*)(mpy_base + 4);

    // execute the bytecode
    uint32_t retval = mp_exec_mpy(mpy_data, mpy_len);

    // check the return value
    if (retval != 0) {
        mp_printf(&mp_plat_print, "retval = %u\n", (uint)retval);
    }

    // deinitialise the MicroPython runtime
    mp_deinit();

    // indicate that the script has completed
    rtems_task_suspend(RTEMS_SELF);

    // destroy this task
    rtems_task_delete(RTEMS_SELF);
}
