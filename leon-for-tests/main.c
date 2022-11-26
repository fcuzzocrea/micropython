/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2015-2016 George Robotics Limited
 *
 * Provided to the European Space Agency as part of the project "Porting of
 * MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
 */

#include <stdio.h>
#include <rtems.h>
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
    _Semaphore_Manager_initialization();
    #endif

    // initialise the timer subsystem
    #if RTEMS_4_8
    _Timer_Manager_initialization(2);
    #elif RTEMS_4
    _Timer_Manager_initialization();
    #endif

    // bring up the datapool

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

#include "py/mphal.h"
#include "leon-common/leonprintf.h"
#include "leon-common/leonutil.h"
#include "leon-common/moddatapool.h"

// this variable defines the location of the externally-loaded .mpy files
#define MPY_MEM_BASE (MICROPY_RTEMS_MPY_MEM_BASE)

// this magic number is used to indicate there are packed .mpy files
#define MPY_MEM_PACKED_HEADER (0x5041434b)

#define DATAPOOL_HEAP_SIZE (4 * 1024)
static uint8_t datapool_heap[DATAPOOL_HEAP_SIZE];

#if MICROPY_RTEMS_USE_TASK_CONSTRUCT
RTEMS_ALIGNED(RTEMS_TASK_STORAGE_ALIGNMENT)
static char mp_task_storage[MICROPY_RTEMS_NUM_TASKS][TASK_STORAGE_SIZE];
static rtems_task_config mp_task_config[MICROPY_RTEMS_NUM_TASKS];
#endif

static int run_scripts(int num_tasks, unsigned int script_offset, unsigned int max_script_index) {
    rtems_id task_id[MICROPY_RTEMS_NUM_TASKS];

    // spawn all worker tasks
    for (int i = 0; i < num_tasks; ++i) {
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
        if (status != RTEMS_SUCCESSFUL) {
            leon_printf("Error creating task #%u: %u\n", i, status);
            return -1;
        }
        unsigned int script_index = script_offset + i;
        if (script_index >= max_script_index) {
            script_index = max_script_index - 1;
        }
        status = rtems_task_start(task_id[i], mp_worker_task, i | script_index << 8);
        if (status != RTEMS_SUCCESSFUL) {
            leon_printf("Error starting task #%u: %u\n", i, status);
            return -1;
        }
    }

    // wait for all scripts to finish
    unsigned int num_tasks_complete = 0;
    for (;;) {
        rtems_task_wake_after(200);
        for (int i = 0; i < num_tasks; ++i) {
            if (rtems_task_is_suspended(task_id[i]) == RTEMS_ALREADY_SUSPENDED) {
                rtems_task_delete(task_id[i]);
                num_tasks_complete += 1;
            }
        }
        if (num_tasks_complete == num_tasks) {
            return 0;
        }
    }
}

rtems_task mp_manager_task(rtems_task_argument ignored) {
    leon_printf("\nMicroPython manager task started\n");

    unsigned int num_tasks = 0;
    unsigned int num_scripts = 0;

    const uint32_t *mpy_mem = (const uint32_t*)MPY_MEM_BASE;
    if (mpy_mem[0] == MPY_MEM_PACKED_HEADER) {
        num_tasks = mpy_mem[1];
        num_scripts = mpy_mem[2];
    }

    leon_printf("Detected %u tasks and %u scripts\n", num_tasks, num_scripts);

    for (unsigned int i = 0; i < num_scripts; i += num_tasks) {
        // Print a message indicating what script(s) are going to be run.
        if (num_tasks == 1) {
            leon_printf("======== Running script %u ========\n", i);
        } else {
            unsigned int max_script = i + num_tasks - 1;
            if (max_script >= num_scripts) {
                max_script = num_scripts - 1;
            }
            leon_printf("======== Running %u tasks with scripts %u-%u ========\n", num_tasks, i, max_script);
        }

        // Initialise the datapool.
        datapool_init(datapool_heap, DATAPOOL_HEAP_SIZE);

        // we must use hexlified output so it isn't modified by the UART
        mp_hal_stdout_enable_hexlify();
        int ret = run_scripts(num_tasks, i, num_scripts);
        mp_hal_stdout_disable_hexlify();

        // Deinitialise the datapool.
        datapool_deinit();

        // Stop if there was an error.
        if (ret < 0) {
            break;
        }
    }

    leon_emu_terminate();
}

/******************************************************************************/
// MicroPython worker task

#include "py/runtime.h"
#include "py/gc.h"
#include "py/stackctrl.h"
#include "py/pystack.h"
#include "mputil.h"
#include "lib/micropython-ulab/code/ulab.h"

// these settings are used to check the C stack for overflow
#define STACK_CHECK_SIZE (MICROPY_RTEMS_STACK_SIZE - 384)
#define STACK_CHECK_BUFFER (32)
#define STACK_CHECK_THRESHOLD (128)

// these settings are used to check the heap for overflow
#define HEAP_PRE (128)
#define HEAP_POST (128)
#define HEAP_SIZE (HEAP_PRE + MICROPY_RTEMS_HEAP_SIZE + HEAP_POST)

static mp_state_ctx_t mp_state_ctx[MICROPY_RTEMS_NUM_TASKS];
static byte mp_heap[MICROPY_RTEMS_NUM_TASKS * HEAP_SIZE];
static mp_obj_t mp_pystack[MICROPY_RTEMS_NUM_TASKS * MICROPY_RTEMS_PYSTACK_WORD_SIZE];

static void pattern_fill(void *p_in, size_t len) {
    uint32_t *p = (uint32_t*)p_in;
    while (len >= 4) {
        *p++ = 0xdea110c8;
        len -= 4;
    }
}

static void *pattern_search(void *p_in, size_t len) {
    uint32_t *p = (uint32_t*)p_in;
    while (len >= 4) {
        if (*p != 0xdea110c8) {
            return p;
        }
        p += 1;
        len -= 4;
    }
    return NULL;
}

rtems_task mp_worker_task(rtems_task_argument rtems_task_arg) {
    // extract the task and script indices from the incoming argument
    unsigned int task_index = rtems_task_arg & 0xff;
    unsigned int script_index = rtems_task_arg >> 8;

    // set the MicroPython context for this task
    mp_state_ptr_set(&mp_state_ctx[task_index]);

    // set value for rtems.script_id() function
    MP_STATE_PORT(rtems_script_id) = mp_obj_new_int(task_index);

    // initialise the stack limit checking
    mp_stack_ctrl_init();
    mp_stack_set_limit(MICROPY_RTEMS_STACK_SIZE - 2048);

    // write a pattern to the stack to check for overflow
    uint32_t dummy;
    byte *stack_start = (byte*)&dummy - STACK_CHECK_SIZE;
    const size_t stack_len = STACK_CHECK_SIZE - STACK_CHECK_BUFFER;
    pattern_fill(stack_start, stack_len);

    // initialise the heap, with a special pattern to check for overflow
    byte *heap_start = &mp_heap[task_index * HEAP_SIZE];
    pattern_fill((uint32_t*)heap_start, HEAP_SIZE);
    gc_init(heap_start + HEAP_PRE, heap_start + HEAP_PRE + MICROPY_RTEMS_HEAP_SIZE);

    // initialise the Python stack
    mp_pystack_init(&mp_pystack[task_index * MICROPY_RTEMS_PYSTACK_WORD_SIZE],
        &mp_pystack[(task_index + 1) * MICROPY_RTEMS_PYSTACK_WORD_SIZE]);

    // initialise the MicroPython runtime
    mp_init();
    ulab_init();

    // get the precompiled bytecode from the fixed address in RAM
    const uint32_t *mpy_mem = (const uint32_t*)MPY_MEM_BASE;
    const byte *mpy_data = (const byte*)MPY_MEM_BASE + mpy_mem[3 + script_index * 2];
    size_t mpy_len = mpy_mem[3 + script_index * 2 + 1];

    // execute the bytecode
    uint32_t retval = mp_exec_mpy(mpy_data, mpy_len);

    // check the return value
    if (retval != 0) {
        mp_printf(&mp_plat_print, "retval = %u\n", (uint)retval);
    }

    // deinitialise the MicroPython runtime
    mp_deinit();

    // check for stack overflow
    {
        void *p = pattern_search(stack_start, stack_len);
        size_t usage = (byte*)&dummy - (byte*)p;
        size_t avail = (byte*)&dummy - stack_start;
        if (usage + STACK_CHECK_THRESHOLD > avail) {
            mp_printf(&mp_plat_print, "stack got too low at %u / %u\n", (uint)usage, (uint)avail);
        }
        //mp_printf(&mp_plat_print, "stack usage: %u / %u\n", (uint)usage, (uint)avail);
    }

    // check for heap overflow
    {
        uint32_t *p = pattern_search((uint32_t*)heap_start, HEAP_PRE);
        if (p != NULL) {
            mp_printf(&mp_plat_print, "pre-heap corruption at %p (heap_start=%p)\n", p, heap_start + HEAP_PRE);
        }
        p = pattern_search((uint32_t*)(heap_start + HEAP_PRE + MICROPY_RTEMS_HEAP_SIZE), HEAP_POST);
        if (p != NULL) {
            mp_printf(&mp_plat_print, "post-heap corruption at %p (heap_end=%p)\n", p, heap_start + HEAP_PRE + MICROPY_RTEMS_HEAP_SIZE);
        }
    }

    // indicate that the script has completed
    rtems_task_suspend(RTEMS_SELF);

    // wait for termination
    for (;;) {
        rtems_task_wake_after(200);
    }
}
