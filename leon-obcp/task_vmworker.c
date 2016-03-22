/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2016 George Robotics Limited
 *
 * Provided to the European Space Agency as part of the project "Porting of
 * MicroPython to LEON platforms", contract number 4000114080/15/NL/FE/as.
 */

/*
 * This file implements the MicroPython VM worker task.
 */

#include <stdio.h>
#include <rtems.h>

#include "py/runtime.h"
#include "py/gc.h"
#include "py/stackctrl.h"
#include "mputil.h"
#include "mpvmmanage.h"

#include "obcp.h"

static mp_state_ctx_t mp_state_ctx[VM_WORKER_NUM_TASKS];
static byte mp_heap[VM_WORKER_NUM_TASKS * VM_WORKER_HEAP_SIZE];

rtems_task obcp_task_mp_worker(rtems_task_argument task_index) {
    printf("MicroPython worker task %d started\n", task_index);

    // set the MicroPython context for this task
    _Thread_Executing->Start.numeric_argument = (uint32_t)&mp_state_ctx[task_index];

    // set value for rtems.script_id() function
    MP_STATE_PORT(rtems_script_id) = mp_obj_new_int(task_index);

    // initialise the stack limit checking
    mp_stack_ctrl_init();
    mp_stack_set_limit(VM_WORKER_STACK_SIZE - 2048);

    // loop forever, loading and executing scripts
    for (;;) {
        const byte *mpy_data;
        size_t mpy_len;
        mp_vm_worker_wait_mpy(&mpy_data, &mpy_len);

        // initialise the heap
        byte *heap_start = &mp_heap[task_index * VM_WORKER_HEAP_SIZE];
        gc_init(heap_start, heap_start + VM_WORKER_HEAP_SIZE);

        // initialise the MicroPython runtime
        mp_init();

        // execute the bytecode
        mp_exec_mpy(mpy_data, mpy_len);

        // deinitialise the MicroPython runtime
        mp_deinit();
    }
}
