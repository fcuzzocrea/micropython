/*
 * This file is part of the MicroPython port to LEON platforms
 * Copyright (c) 2017 George Robotics Limited
 */
#ifndef INCLUDED_LEON_COMMON_RTEMSSEM_H
#define INCLUDED_LEON_COMMON_RTEMSSEM_H

#if RTEMS_4_8_EDISOFT || RTEMS_6

// RTEMS 4.8 Edisoft doesn't provide POSIX semaphores so we provide a thin
// abstraction layer on top of RTEMS semaphores.

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
    #if RTEMS_4_8
    _Semaphore_Manager_initialization(max_sem);
    #elif RTEMS_4
    (void)max_sem;
    _Semaphore_Manager_initialization();
    #endif
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

#else

// This version of RTEMS supports POSIX semaphores so use them.

#include <errno.h>
#include <sched.h>
#include <semaphore.h>
#if RTEMS_4
#include <rtems/posix/semaphore.h>
#endif
#if RTEMS_5
#include <rtems/rtems/clock.h>
#endif

typedef sem_t mp_sem_t;

static inline void mp_sem_init(uint32_t max_sem) {
    (void)max_sem;
    #if RTEMS_4_8
    _POSIX_Semaphore_Manager_initialization(max_sem);
    #elif RTEMS_4
    (void)max_sem;
    _POSIX_Semaphore_Manager_initialization();
    #endif
}

static inline int mp_sem_new(mp_sem_t *sem, uint32_t value, uint32_t id) {
    (void)id; // unused for POSIX implementation of semaphores
    int ret = sem_init(sem, 0, value);
    if (ret == 0) {
        return 0;
    } else {
        return errno;
    }
}

static inline int mp_sem_post(mp_sem_t *sem) {
    int ret = sem_post(sem);
    if (ret == 0) {
        return 0;
    } else {
        return errno;
    }
}

static inline int mp_sem_wait(mp_sem_t *sem) {
    int ret = sem_wait(sem);
    if (ret == 0) {
        return 0;
    } else {
        return errno;
    }
}

static inline int mp_sem_trywait(mp_sem_t *sem) {
    int ret = sem_trywait(sem);
    if (ret == 0) {
        return 0;
    } else {
        return errno;
    }
}

static inline int mp_sem_timedwait(mp_sem_t *sem, uint32_t timeout_ticks) {
    #if RTEMS_5
    uint64_t timeout_ns = 1000000000ULL * (uint64_t)timeout_ticks / (uint64_t)rtems_clock_get_ticks_per_second();
    struct timespec timespec;
    timespec.tv_sec = timeout_ns / 1000000000ULL;
    timespec.tv_nsec = timeout_ns % 1000000000ULL;
    int ret = sem_timedwait(sem, &timespec);
    if (ret == 0) {
        return 0;
    } else {
        return errno;
    }
    #else
    // Note: sem_timedwait doesn't seem to work in RTEMS 4, it always times out,
    // so instead we use a loop which keeps trying to wait on the semaphore.
    rtems_interval ticks_start;
    rtems_status_code status = rtems_clock_get(RTEMS_CLOCK_GET_TICKS_SINCE_BOOT, &ticks_start);
    if (status != RTEMS_SUCCESSFUL) {
        return EINVAL;
    }
    for (;;) {
        int ret = sem_trywait(sem);
        if (ret == 0) {
            // success
            return 0;
        } else if (errno != EAGAIN) {
            // error
            return errno;
        }
        rtems_interval ticks;
        status = rtems_clock_get(RTEMS_CLOCK_GET_TICKS_SINCE_BOOT, &ticks);
        if (status != RTEMS_SUCCESSFUL) {
            return EINVAL;
        }
        if (ticks - ticks_start >= timeout_ticks) {
            // timeout waiting
            return ETIMEDOUT;
        }
        status = rtems_task_wake_after(5);
        if (status != RTEMS_SUCCESSFUL) {
            return EINVAL;
        }
    }
    return EINVAL;
    #endif
}

#endif

#endif // INCLUDED_LEON_COMMON_RTEMSSEM_H
