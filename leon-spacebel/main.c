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
#define CONFIGURE_MAXIMUM_TASKS (13)
#define CONFIGURE_MAXIMUM_SEMAPHORES (4)
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES (4)
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_EXTRA_TASK_STACKS (20 * RTEMS_MINIMUM_STACK_SIZE)

rtems_task Init(rtems_task_argument argument);
rtems_task mp_manager_task(rtems_task_argument unused);
rtems_task mp_worker_task(rtems_task_argument unused);

#include <rtems/confdefs.h>
#include "leon-common/sparcisr.h"
#include "leon-common/moddatapool.h"

#define MICROPY_RTEMS_TASK_ATTRIBUTES (RTEMS_APPLICATION_TASK | RTEMS_FLOATING_POINT)
#define MICROPY_RTEMS_STACK_SIZE (RTEMS_MINIMUM_STACK_SIZE * 3)
#define MICROPY_RTEMS_HEAP_SIZE (48 * 1024)
#define MICROPY_RTEMS_NUM_TASKS (10)
#define MICROPY_RTEMS_PYSTACK_WORD_SIZE (512)

#define DATAPOOL_HEAP_SIZE (4 * 1024)
static uint8_t datapool_heap[DATAPOOL_HEAP_SIZE];

/******************************************************************************/
// RTEMS initialisation task
// this task runs at highest priority and is non-preemptive

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
    #else
    //_Message_queue_Manager_initialization();
    //_Semaphore_Manager_initialization();
    #endif

    // initialise the timer subsystem
    #if RTEMS_4_8
    _Timer_Manager_initialization(2);
    #else
    //_Timer_Manager_initialization();
    #endif

    // bring up the datapool
    datapool_init(datapool_heap, DATAPOOL_HEAP_SIZE);

    // start the manager task to do the rest of the work
    rtems_name task_name = rtems_build_name('M', 'P', 'M', 'A');
    rtems_id task_id;
    rtems_status_code status;
    status = rtems_task_create(
        task_name, 1, RTEMS_MINIMUM_STACK_SIZE, RTEMS_DEFAULT_MODES,
        MICROPY_RTEMS_TASK_ATTRIBUTES, &task_id
    );
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

// these variables define the location of the externally-loaded .mpy files
#define MPY_MEM_BASE   (0x40200000)
#define MPY_MEM_STRIDE (0x00010000)

#include "py/mphal.h"
#include "leon-common/leonprintf.h"

// this function is used as a hook to set a breakpoint to terminate emu
void emu_terminate(void) {
    leon_printf("emu_terminate\n");
}

rtems_task mp_manager_task(rtems_task_argument ignored) {
    leon_printf("\nMicroPython manager task started\n");

    // detect the number of tasks needed by looking for valid scripts
    int num_tasks = 0;
    for (int i = 0; i < MICROPY_RTEMS_NUM_TASKS; ++i) {
        const void *mpy_base = (const void*)(MPY_MEM_BASE + MPY_MEM_STRIDE * i);
        size_t mpy_len = *(const uint32_t*)mpy_base;
        const uint8_t *mpy_data = (const uint8_t*)(mpy_base + 4);
        if (mpy_len > 0 && mpy_data[0] == 'M') {
            num_tasks += 1;
        } else {
            break;
        }
    }
    leon_printf("Detected %u scripts\n", num_tasks);

    // we must use hexlified output so it isn't modified by the UART
    mp_hal_stdout_enable_hexlify();

    rtems_name task_name[MICROPY_RTEMS_NUM_TASKS];
    rtems_id task_id[MICROPY_RTEMS_NUM_TASKS];

    // spawn all worker tasks
    for (int i = 0; i < num_tasks; ++i) {
        rtems_status_code status;
        task_name[i] = rtems_build_name('M', 'P', '0', '0' + i);
        status = rtems_task_create(
            task_name[i], 1, MICROPY_RTEMS_STACK_SIZE, RTEMS_DEFAULT_MODES,
            MICROPY_RTEMS_TASK_ATTRIBUTES, &task_id[i]
        );
        if (status != RTEMS_SUCCESSFUL) {
            leon_printf("Error creating task #%u: %u\n", i, status);
            emu_terminate();
        }
        status = rtems_task_start(task_id[i], mp_worker_task, i);
        if (status != RTEMS_SUCCESSFUL) {
            leon_printf("Error starting task #%u: %u\n", i, status);
            emu_terminate();
        }
    }

    // wait for all scripts to finish
    for (;;) {
        rtems_task_wake_after(200);
        int num_tasks_complete = 0;
        for (int i = 0; i < num_tasks; ++i) {
            uint32_t note;
            rtems_task_get_note(task_id[i], RTEMS_NOTEPAD_0, &note);
            if (note != 0) {
                num_tasks_complete += 1;
            }
        }
        if (num_tasks_complete == num_tasks) {
            mp_hal_stdout_disable_hexlify();
            emu_terminate();
        }
    }
}

/******************************************************************************/
// MicroPython worker task

#include "py/runtime.h"
#include "py/gc.h"
#include "py/stackctrl.h"
#include "py/pystack.h"
#include "mputil.h"

static mp_state_ctx_t mp_state_ctx[MICROPY_RTEMS_NUM_TASKS];
static byte mp_heap[MICROPY_RTEMS_NUM_TASKS * MICROPY_RTEMS_HEAP_SIZE];
static mp_obj_t mp_pystack[MICROPY_RTEMS_NUM_TASKS * MICROPY_RTEMS_PYSTACK_WORD_SIZE];

rtems_task mp_worker_task(rtems_task_argument task_index) {
    // set the MicroPython context for this task
    _Thread_Executing->Start.numeric_argument = (uint32_t)&mp_state_ctx[task_index];

    // set value for rtems.script_id() function
    MP_STATE_PORT(rtems_script_id) = mp_obj_new_int(task_index);

    // initialise the stack limit checking
    mp_stack_ctrl_init();
    mp_stack_set_limit(MICROPY_RTEMS_STACK_SIZE - 2048);

    // initialise the heap
    byte *heap_start = &mp_heap[task_index * MICROPY_RTEMS_HEAP_SIZE];
    gc_init(heap_start, heap_start + MICROPY_RTEMS_HEAP_SIZE);

    // initialise the Python stack
    mp_pystack_init(&mp_pystack[task_index * MICROPY_RTEMS_PYSTACK_WORD_SIZE],
        &mp_pystack[(task_index + 1) * MICROPY_RTEMS_PYSTACK_WORD_SIZE]);

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
    rtems_task_set_note(RTEMS_SELF, RTEMS_NOTEPAD_0, 1);

    // wait for termination
    for (;;) {
        rtems_task_wake_after(200);
    }
}