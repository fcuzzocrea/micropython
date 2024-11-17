/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2017 George Robotics Limited
 */
#ifndef INCLUDED_MPFS_COMMON_RTEMSSEM_H
#define INCLUDED_MPFS_COMMON_RTEMSSEM_H

#include <rtems.h>
#include <rtems/rtems/sem.h>

// RTEMS 4.8 Edisoft doesn't have a working errno.h so we provide definitions
// for just the constants we need (and their values don't really matter).
#ifndef EINVAL
#define EINVAL (22)
#endif
#ifndef ETIMEDOUT
#define ETIMEDOUT (116)
#endif

typedef rtems_id mp_sem_t;

static inline void mp_sem_init(uint32_t max_sem) {

}

static inline int mp_sem_new(mp_sem_t *sem, uint32_t value, uint32_t id) {
    rtems_name name = rtems_build_name('R', 'S', id & 0xff, (id >> 8) & 0xff);
    rtems_status_code ret = rtems_semaphore_create(name, value, 0, 0, sem);
    if (ret == RTEMS_SUCCESSFUL) {
        return 0;
    } else {
        return EINVAL;
    }
}

static inline int mp_sem_post(mp_sem_t *sem) {
    rtems_status_code ret = rtems_semaphore_release(*sem);
    if (ret == RTEMS_SUCCESSFUL) {
        return 0;
    } else {
        return EINVAL;
    }
}

static inline int mp_sem_wait(mp_sem_t *sem) {
    rtems_status_code ret = rtems_semaphore_obtain(*sem, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
    if (ret == RTEMS_SUCCESSFUL) {
        return 0;
    } else {
        return EINVAL;
    }
}

static inline int mp_sem_trywait(mp_sem_t *sem) {
    rtems_status_code ret = rtems_semaphore_obtain(*sem, RTEMS_NO_WAIT, RTEMS_NO_TIMEOUT);
    if (ret == RTEMS_SUCCESSFUL) {
        return 0;
    } else {
        return EINVAL;
    }
}

static inline int mp_sem_timedwait(mp_sem_t *sem, uint32_t timeout_ticks) {
    rtems_status_code ret = rtems_semaphore_obtain(*sem, RTEMS_WAIT, timeout_ticks);
    if (ret == RTEMS_SUCCESSFUL) {
        return 0;
    } else if (ret == RTEMS_TIMEOUT) {
        return ETIMEDOUT;
    } else {
        return EINVAL;
    }
}

#endif // INCLUDED_MPFS_COMMON_RTEMSSEM_H
