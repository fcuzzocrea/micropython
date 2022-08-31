/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2015-2022 George Robotics Limited
 *
 * Provided to the European Space Agency as part of the project "Evolutions
 * of MicroPython for LEON", contract number 4000137198/22/NL/MGu/kk.
 */

#include "rtems_config.h"
#include "leon-common/rtems_util.h"

#if RTEMS_6
// RTEMS 6 requires this.
int __errno;
#endif

#if MICROPY_RTEMS_USE_TASK_CONSTRUCT
void mp_rtems_task_config(rtems_task_config *config, rtems_name name, void *storage_area, size_t storage_size) {
    config->name = name;
    config->initial_priority = 1;
    config->storage_area = storage_area;
    config->storage_size = storage_size;
    config->maximum_thread_local_storage_size = MAX_TLS_SIZE;
    config->storage_free = NULL;
    config->initial_modes = RTEMS_DEFAULT_MODES;
    config->attributes = MICROPY_RTEMS_TASK_ATTRIBUTES;
}
#endif
