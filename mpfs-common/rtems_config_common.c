/*
 * This file is part of the MicroPython port to MPFS platforms
 *
 * Based on LEON port originally made by George Robotics
 *
 * Copyright (c) 2023 Francescodario Cuzzocrea <bosconovic@gmail.com>
 */

#include <rtems.h>
#include "rtems_util.h"

// Configuration for: RTEMS 6.
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER
#define CONFIGURE_MAXIMUM_PROCESSORS 4
#define CONFIGURE_MAXIMUM_TASKS (1 + CONFIGURE_MAXIMUM_PROCESSORS)
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT
#define CONFIGURE_MAXIMUM_BARRIERS 3
#ifndef CONFIGURE_MAXIMUM_MESSAGE_QUEUES
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES 3
#endif
#define CONFIGURE_MAXIMUM_PARTITIONS 3
#define CONFIGURE_MAXIMUM_PERIODS 3
#ifndef CONFIGURE_MAXIMUM_SEMAPHORES
#define CONFIGURE_MAXIMUM_SEMAPHORES 3
#endif
// edit - we do not use task construct
#define CONFIGURE_MINIMUM_TASKS_WITH_USER_PROVIDED_STORAGE 1
#define CONFIGURE_MAXIMUM_TIMERS 3
#define CONFIGURE_MAXIMUM_USER_EXTENSIONS 3
// needed for uart
//#define CONFIGURE_APPLICATION_DISABLE_FILESYSTEM
#define CONFIGURE_DISABLE_NEWLIB_REENTRANCY
#define CONFIGURE_INIT_TASK_ATTRIBUTES MICROPY_RTEMS_TASK_ATTRIBUTES
// edit - we do not use task construct
//#define CONFIGURE_INIT_TASK_CONSTRUCT_STORAGE_SIZE TASK_STORAGE_SIZE
#define CONFIGURE_INIT_TASK_INITIAL_MODES RTEMS_DEFAULT_MODES
// needed for uart
//#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS 0
#define CONFIGURE_MAXIMUM_THREAD_LOCAL_STORAGE_SIZE MAX_TLS_SIZE
#define CONFIGURE_MICROSECONDS_PER_TICK 1000

#include <rtems/confdefs.h>

// Thread-local-state accessor functions.
void *mp_state_ptr(void);
void mp_state_ptr_set(void *value);

void *mp_state_ptr(void) {
    return ((void*)_Thread_Get_executing()->Start.Entry.Kinds.Numeric.argument);
}

void mp_state_ptr_set(void *value) {
    _Thread_Get_executing()->Start.Entry.Kinds.Numeric.argument = (uintptr_t) value;
}
