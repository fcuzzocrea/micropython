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
#define CONFIGURE_MAXIMUM_TASKS (10)
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_EXTRA_TASK_STACKS (20 * RTEMS_MINIMUM_STACK_SIZE)

rtems_task Init(rtems_task_argument argument);
rtems_task mp_manager_task(rtems_task_argument unused);
rtems_task mp_worker_task(rtems_task_argument unused);

#include <rtems/confdefs.h>
#include "obcp.h"
#include "leon-common/mpvmmanage.h"
#include "leon-common/moddatapool.h"

static uint8_t datapool_heap[DATAPOOL_HEAP_SIZE];
static double start_time;

void set_start_time(void) {
    // can't do FP in Init task
    rtems_clock_time_value t;
    rtems_clock_get(RTEMS_CLOCK_GET_TIME_VALUE, &t);
    start_time = t.seconds + 1e-6 * t.microseconds;
}

double get_time(void) {
    rtems_clock_time_value t;
    rtems_clock_get(RTEMS_CLOCK_GET_TIME_VALUE, &t);
    return t.seconds + 1e-6 * t.microseconds - start_time;
}

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
    _Message_queue_Manager_initialization(6);

    // initialise the timer subsystem
    _Timer_Manager_initialization(2);

    // initialise POSIX components
    _POSIX_Semaphore_Manager_initialization(VM_WORKER_NUM_TASKS * 2);

    // print a message to indicate start-up
    printf("\nOBCP program started\n");

    // create the TM queue
    {
        rtems_id q_id;
        rtems_name name = rtems_build_name('T', 'L', 'M', 'Q');
        rtems_status_code status = rtems_message_queue_create(name, 4, sizeof(TM_t), RTEMS_DEFAULT_ATTRIBUTES, &q_id);
        if (status != RTEMS_SUCCESSFUL) {
            // error
        }
    }

    // create the TC queue
    {
        rtems_id q_id;
        rtems_name name = rtems_build_name('T', 'C', 'Q', ' ');
        rtems_status_code status = rtems_message_queue_create(name, 4, sizeof(TC_t), RTEMS_DEFAULT_ATTRIBUTES, &q_id);
        if (status != RTEMS_SUCCESSFUL) {
            // error
        }
    }

    // create the power threshold queue
    {
        rtems_id q_id;
        rtems_name name = rtems_build_name('P', 'W', 'T', 'H');
        rtems_status_code status = rtems_message_queue_create(name, 4, sizeof(TC_t), RTEMS_DEFAULT_ATTRIBUTES, &q_id);
        if (status != RTEMS_SUCCESSFUL) {
            // error
        }
    }

    // bring up the datapool
    {
        datapool_init(datapool_heap, DATAPOOL_HEAP_SIZE);

        datapool_t dp;
        datapool_create("DATAPOOL", &dp);

        // initial values for datapool entries
        static const double init_volt_thresh[20] =
            {10,13,10,13,10,13,10,13,10,13,10,13,10,13,10,13,10,13,10,13};
        static const double init_curr_thresh[20] =
            {30e-3,40e-3,30e-3,40e-3,30e-3,40e-3,30e-3,40e-3,30e-3,40e-3,
                30e-3,40e-3,30e-3,40e-3,30e-3,40e-3,30e-3,40e-3,30e-3,40e-3};
        static const double init_temp[30] =
            {273,273,273,273,273,273,273,273,273,273,273,273,273,273,273,
                273,273,273,273,273,273,273,273,273,273,273,273,273,273,273};
        static const double init_temp_thresh[60] =
            {263,283,263,283,263,283,263,283,263,283,263,283,263,283,263,283,
                263,283,263,283,263,283,263,283,263,283,263,283,263,283,263,283,
                263,283,263,283,263,283,263,283,263,283,263,283,263,283,263,283,
                263,283,263,283,263,283,263,283,263,283,263,283};

        datapool_set_buf(dp, K_DP_VOLT_THRESH_10, init_volt_thresh, 20 * sizeof(double));
        datapool_set_buf(dp, K_DP_CURR_THRESH_10, init_curr_thresh, 20 * sizeof(double));
        datapool_set_buf(dp, K_DP_TEMP_VAL_30, init_temp, 30 * sizeof(double));
        datapool_set_buf(dp, K_DP_TEMP_THRESH_30, init_temp_thresh, 60 * sizeof(double));

        // from now on the datapool is fixed, nothing can be added (but values can be changed)
        datapool_heap_lock();
    }

    // spawn all MicroPython worker tasks
    for (int i = 0; i < VM_WORKER_NUM_TASKS; ++i) {
        rtems_status_code status;
        rtems_name task_name = rtems_build_name('M', 'P', 'W', '0' + i);
        rtems_id task_id;
        status = rtems_task_create(
            task_name, 1, VM_WORKER_STACK_SIZE, RTEMS_DEFAULT_MODES,
            VM_WORKER_TASK_ATTRIBUTES, &task_id
        );
        if (status != RTEMS_SUCCESSFUL) {
            printf("Error creating VM task #%u: %u\n", i, status);
            return;
        }
        mp_vm_worker_init(get_vm_worker_ctx(i));
        status = rtems_task_start(task_id, obcp_task_mp_worker, i);
        if (status != RTEMS_SUCCESSFUL) {
            printf("Error starting VM task #%u: %u\n", i, status);
            return;
        }
    }

    // start the TM task
    {
        rtems_name task_name = rtems_build_name('O', 'B', 'T', 'M');
        rtems_id task_id;
        rtems_status_code status;
        status = rtems_task_create(
            task_name, 1, NATIVE_TASK_STACK_SIZE, RTEMS_DEFAULT_MODES,
            NATIVE_TASK_ATTRIBUTES, &task_id
        );
        if (status != RTEMS_SUCCESSFUL) {
            printf("Error creating TM task: %u\n", status);
            return;
        }
        status = rtems_task_start(task_id, obcp_task_tm, 0);
        if (status != RTEMS_SUCCESSFUL) {
            printf("Error starting TM task: %u\n", status);
            return;
        }
    }

    // start the TC task
    {
        rtems_name task_name = rtems_build_name('O', 'B', 'T', 'C');
        rtems_id task_id;
        rtems_status_code status;
        status = rtems_task_create(
            task_name, 1, NATIVE_TASK_STACK_SIZE, RTEMS_DEFAULT_MODES,
            NATIVE_TASK_ATTRIBUTES, &task_id
        );
        if (status != RTEMS_SUCCESSFUL) {
            printf("Error creating TC task: %u\n", status);
            return;
        }
        status = rtems_task_start(task_id, obcp_task_tc, 0);
        if (status != RTEMS_SUCCESSFUL) {
            printf("Error starting TC task: %u\n", status);
            return;
        }
    }

    // start the TEMP task
    {
        rtems_name task_name = rtems_build_name('O', 'B', 'T', 'P');
        rtems_id task_id;
        rtems_status_code status;
        status = rtems_task_create(
            task_name, 1, NATIVE_TASK_STACK_SIZE, RTEMS_DEFAULT_MODES,
            NATIVE_TASK_ATTRIBUTES, &task_id
        );
        if (status != RTEMS_SUCCESSFUL) {
            printf("Error creating TEMP task: %u\n", status);
            return;
        }
        status = rtems_task_start(task_id, obcp_task_temp, 0);
        if (status != RTEMS_SUCCESSFUL) {
            printf("Error starting TEMP task: %u\n", status);
            return;
        }
    }

    // start the GND task
    {
        rtems_name task_name = rtems_build_name('O', 'B', 'T', 'G');
        rtems_id task_id;
        rtems_status_code status;
        status = rtems_task_create(
            task_name, 1, NATIVE_TASK_STACK_SIZE, RTEMS_DEFAULT_MODES,
            NATIVE_TASK_ATTRIBUTES, &task_id
        );
        if (status != RTEMS_SUCCESSFUL) {
            printf("Error creating GND task: %u\n", status);
            return;
        }
        status = rtems_task_start(task_id, obcp_task_ground, 0);
        if (status != RTEMS_SUCCESSFUL) {
            printf("Error starting GND task: %u\n", status);
            return;
        }
    }

    // delete current task so that all child tasks can start running
    rtems_task_delete(RTEMS_SELF);
}
